#include "enemies/Tornado.hpp"
#include "Player.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cstdlib>

extern Player* g_player;

static float randRange(float lo, float hi) {
    return lo + (static_cast<float>(std::rand()) / RAND_MAX) * (hi - lo);
}

// ---------------------------------------------------------------
Tornado::Tornado(sf::Vector2f pos)
    : FlyngFoogaFoog(pos)   // hitsToEncase will be overridden by takeAttackHit
    , m_walkTexLoaded(false), m_walkTexFrame(0), m_walkTexTimer(0.f)
    , m_flyFramesLoaded(false), m_flyFrame(0), m_flyFrameTimer(0.f)
    , m_leftLoaded(false), m_rightLoaded(false), m_jumpLoaded2(false)
    , m_turnLoaded(false), m_isTurning(false), m_turnFrame(0), m_turnTimer(0.f)
    , m_prevFacingRight(false)
    , m_throwLoaded(false), m_isThrowing(false), m_throwFrame(0), m_throwTimer(0.f)
    , m_knifeSpawnedThisCycle(false)
    , m_enc25Loaded(false), m_enc50Loaded(false)
    , m_knifeTimer(0.f), m_knifeCooldown(0.f)
    , m_flySpeed(100.f)
{
    // Override base fooga assets with tornado-specific ones
    loadEnemyAssets(
        "assets/sprites/tornado_walk_frame1.png",     // idle
        "assets/sprites/tornado_walk_frame1.png",     // trapped (snowball body)
        "assets/sprites/tornado_unleashed75.png",
        "assets/sprites/tornado_unleashed50.png",
        "assets/sprites/tornado_unleashed25.png",
        "assets/sprites/tornado_encased25.png",       // encase50 slot (unused directly)
        "assets/sprites/snow_encase_100.png",
        "assets/sprites/snow_escape_75.png",
        "assets/sprites/snow_escape_50.png",
        "assets/sprites/snow_escape_25.png"
    );

    // Encasement overlays
    auto tryTex = [&](sf::Texture& t, bool& flag, const char* p) {
        if (std::FILE* f = std::fopen(p, "rb")) { std::fclose(f); flag = t.loadFromFile(p); }
        else { std::cerr << "[Tornado] missing: " << p << "\n"; flag = false; }
    };
    tryTex(m_enc25Tex, m_enc25Loaded, "assets/sprites/tornado_encased25.png");
    tryTex(m_enc50Tex, m_enc50Loaded, "assets/sprites/tornado_encased50.png");

    // Walk frames
    m_walkTexLoaded = true;
    for (int i = 0; i < 3; ++i) {
        std::string p = "assets/sprites/tornado_walk_frame" + std::to_string(i+1) + ".png";
        if (std::FILE* f = std::fopen(p.c_str(), "rb")) {
            std::fclose(f);
            if (!m_walkTextures[i].loadFromFile(p)) m_walkTexLoaded = false;
        } else { std::cerr << "[Tornado] missing: " << p << "\n"; m_walkTexLoaded = false; }
    }

    // Fly frames (tornado_frame1/2)
    m_flyFramesLoaded = true;
    for (int i = 0; i < 2; ++i) {
        std::string p = "assets/sprites/tornado_frame" + std::to_string(i+1) + ".png";
        if (std::FILE* f = std::fopen(p.c_str(), "rb")) {
            std::fclose(f);
            if (!m_flyFrames[i].loadFromFile(p)) m_flyFramesLoaded = false;
        } else { std::cerr << "[Tornado] missing: " << p << "\n"; m_flyFramesLoaded = false; }
    }

    // Directional / jump
    tryTex(m_leftTex,  m_leftLoaded,  "assets/sprites/tornado_left.png");
    tryTex(m_rightTex, m_rightLoaded, "assets/sprites/tornado_right.png");
    tryTex(m_jumpTex2, m_jumpLoaded2, "assets/sprites/tornado_jump.png");

    // Turn frames
    m_turnLoaded = true;
    for (int i = 0; i < 4; ++i) {
        std::string p = "assets/sprites/tornado_turning_frame" + std::to_string(i+1) + ".png";
        if (std::FILE* f = std::fopen(p.c_str(), "rb")) {
            std::fclose(f);
            if (!m_turnFrames[i].loadFromFile(p)) m_turnLoaded = false;
        } else { std::cerr << "[Tornado] missing: " << p << "\n"; m_turnLoaded = false; }
    }

    // Throw frames
    m_throwLoaded = true;
    for (int i = 0; i < 2; ++i) {
        std::string p = "assets/sprites/tornado_throw_frame" + std::to_string(i+1) + ".png";
        if (std::FILE* f = std::fopen(p.c_str(), "rb")) {
            std::fclose(f);
            if (!m_throwFrames[i].loadFromFile(p)) m_throwLoaded = false;
        } else { std::cerr << "[Tornado] missing: " << p << "\n"; m_throwLoaded = false; }
    }

    // Force 3-hit encasement
    m_hitsToEncase = 3;
    m_hitsTaken    = 0;
    setOneHitEncase(false);

    randomizeFlySpeed();
    randomizeKnifeCooldown();
    m_prevFacingRight = m_facingRight;
}

// ---------------------------------------------------------------
void Tornado::randomizeFlySpeed() {
    m_flySpeed = randRange(MIN_FLY_SPEED, MAX_FLY_SPEED);
}

void Tornado::randomizeKnifeCooldown() {
    m_knifeCooldown = randRange(MIN_KNIFE_CD, MAX_KNIFE_CD);
    m_knifeTimer    = 0.f;
}

// ---------------------------------------------------------------
Tornado::KnifeRequest Tornado::getAndClearKnifeSpawn() {
    KnifeRequest r = m_knifeRequest;
    m_knifeRequest.pending = false;
    return r;
}

// ---------------------------------------------------------------
// update — adds knife timer and turn detection on top of fooga logic
// ---------------------------------------------------------------
void Tornado::update(float dt) {
    // Detect direction change → trigger turn animation
    if (m_state == State::Alive && m_facingRight != m_prevFacingRight) {
        if (!m_isTurning) {
            m_isTurning  = true;
            m_turnFrame  = 0;
            m_turnTimer  = 0.f;
        }
        m_prevFacingRight = m_facingRight;
    }

    // Knife throw timer (only when alive)
    if (m_state == State::Alive && g_player) {
        m_knifeTimer += dt;
        if (m_knifeTimer >= m_knifeCooldown && !m_isThrowing) {
            m_isThrowing             = true;
            m_throwFrame             = 0;
            m_throwTimer             = 0.f;
            m_knifeSpawnedThisCycle  = false;
            m_knifeTimer             = 0.f;
            randomizeKnifeCooldown();
        }
    }

    // Spawn knife at frame 1 of throw animation (mid-throw)
    if (m_isThrowing && m_throwFrame == 1 && !m_knifeSpawnedThisCycle && g_player) {
        sf::Vector2f spawnPos = {position.x + 28.f, position.y + 20.f};
        sf::Vector2f dir = g_player->getPosition() - spawnPos;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 1.f) { dir.x /= len; dir.y /= len; }
        m_knifeRequest = {true, spawnPos, dir};
        m_knifeSpawnedThisCycle = true;
    }

    // Override fly speed in fooga's velocity — done by modifying velocity
    // directly after fooga update runs its AI. We call base update then fix speed.
    FlyngFoogaFoog::update(dt);

    // Re-normalize fly velocity to our randomized speed
    if (m_state == State::Alive && isFlying()) {
        float len = std::sqrt(velocity.x*velocity.x + velocity.y*velocity.y);
        if (len > 1.f) {
            velocity.x = (velocity.x / len) * m_flySpeed;
            velocity.y = (velocity.y / len) * m_flySpeed;
        }
    }

    // Re-randomize fly speed each time fooga switches from walk→fly
    // Detect: isFlying() just became true
    static bool s_wasFlying = false;
    if (isFlying() && !s_wasFlying) randomizeFlySpeed();
    s_wasFlying = isFlying();
}

// ---------------------------------------------------------------
// takeAttackHit — 3-hit encasement
// ---------------------------------------------------------------
void Tornado::takeAttackHit() {
    if (m_state != State::Alive &&
        m_state != State::PartialEncase &&
        m_state != State::Escaping75 &&
        m_state != State::Escaping50 &&
        m_state != State::Escaping25) return;

    if (m_state == State::Escaping75 || m_state == State::Escaping50) {
        m_state = State::Snowballed; m_stateTimer = 3.f;
        m_hitsTaken = m_hitsToEncase; velocity = {0.f,0.f}; return;
    }
    if (m_state == State::Escaping25) {
        m_state = State::PartialEncase; m_hitsTaken = 2;
        m_stateTimer = 1.5f; velocity = {0.f,0.f}; return;
    }

    ++m_hitsTaken;
    if (m_hitsTaken >= m_hitsToEncase) {
        m_state = State::Snowballed; m_stateTimer = 3.f; velocity = {0.f,0.f};
    } else {
        m_state = State::PartialEncase;
        m_stateTimer = 1.5f; velocity = {0.f,0.f};
    }
}

// ---------------------------------------------------------------
// updateStateTimers — same 3-hit decay as BotomBlue
// ---------------------------------------------------------------
void Tornado::updateStateTimers(float dt) {
    if (m_state == State::PartialEncase) {
        m_stateTimer -= dt;
        if (m_stateTimer <= 0.f) {
            --m_hitsTaken;
            if (m_hitsTaken <= 0) { m_hitsTaken=0; m_state=State::Alive; m_stateTimer=0.f; }
            else m_stateTimer = 1.5f;
        }
        return;
    }
    Enemy::updateStateTimers(dt);
}

// ---------------------------------------------------------------
// applyStateSprite
// ---------------------------------------------------------------
void Tornado::applyStateSprite() {
    m_overlayVisible = false;
    m_bodySprite.setColor(sf::Color::White);

    switch (m_state) {
        case State::Alive:
            break; // animation handles
        case State::PartialEncase:
            if (m_hitsTaken == 1 && m_enc25Loaded)
                m_bodySprite.setTexture(m_enc25Tex, true);
            else if (m_hitsTaken >= 2 && m_enc50Loaded)
                m_bodySprite.setTexture(m_enc50Tex, true);
            break;
        case State::Snowballed:
            if (m_trappedLoaded) m_bodySprite.setTexture(m_trappedTexture, true);
            if (m_snowEncase100Loaded) {
                m_overlaySprite.setTexture(m_snowEncase100Texture, true);
                m_overlayVisible = true;
            }
            break;
        case State::Escaping75:
            if (m_unleash1Loaded) m_bodySprite.setTexture(m_unleash1Texture, true); break;
        case State::Escaping50:
            if (m_unleash2Loaded) m_bodySprite.setTexture(m_unleash2Texture, true); break;
        case State::Escaping25:
            if (m_unleash3Loaded) m_bodySprite.setTexture(m_unleash3Texture, true); break;
        case State::Rolling:
            if (m_trappedLoaded) m_bodySprite.setTexture(m_trappedTexture, true);
            if (m_snowEncase100Loaded) {
                m_overlaySprite.setTexture(m_snowEncase100Texture, true);
                m_overlayVisible = true;
            }
            break;
        case State::Dead: break;
    }
}

// ---------------------------------------------------------------
// updateAnimation
// ---------------------------------------------------------------
void Tornado::updateAnimation(float dt) {
    if (m_state != State::Alive) return;

    // Throw animation takes priority
    if (m_isThrowing) {
        m_throwTimer += dt;
        if (m_throwTimer >= THROW_FRAME_TIME) {
            m_throwTimer -= THROW_FRAME_TIME;
            ++m_throwFrame;
            if (m_throwFrame >= 2) { m_isThrowing = false; m_throwFrame = 0; }
        }
        if (m_throwLoaded && m_throwFrame < 2)
            m_bodySprite.setTexture(m_throwFrames[m_throwFrame], true);
        return;
    }

    // Turn animation
    if (m_isTurning) {
        m_turnTimer += dt;
        if (m_turnTimer >= TURN_FRAME_TIME) {
            m_turnTimer -= TURN_FRAME_TIME;
            ++m_turnFrame;
            if (m_turnFrame >= 4) { m_isTurning = false; m_turnFrame = 0; }
        }
        if (m_turnLoaded && m_turnFrame < 4)
            m_bodySprite.setTexture(m_turnFrames[m_turnFrame], true);
        return;
    }

    // Flying
    if (isFlying()) {
        if (m_flyFramesLoaded) {
            m_flyFrameTimer += dt;
            if (m_flyFrameTimer >= FLY_FRAME_TIME) {
                m_flyFrameTimer -= FLY_FRAME_TIME;
                m_flyFrame = (m_flyFrame + 1) % 2;
            }
            m_bodySprite.setTexture(m_flyFrames[m_flyFrame], true);
        }
        return;
    }

    // Walking / idle — use directional or walk frames
    if (!m_onGround) {
        if (m_jumpLoaded2) m_bodySprite.setTexture(m_jumpTex2, true);
        return;
    }
    if (velocity.x != 0.f && m_walkTexLoaded) {
        m_walkTexTimer += dt;
        if (m_walkTexTimer >= WALK_TEX_TIME) {
            m_walkTexTimer -= WALK_TEX_TIME;
            m_walkTexFrame = (m_walkTexFrame + 1) % 3;
        }
        // Use left/right directional if available, else walk frames
        if (m_facingRight && m_rightLoaded)
            m_bodySprite.setTexture(m_rightTex, true);
        else if (!m_facingRight && m_leftLoaded)
            m_bodySprite.setTexture(m_leftTex, true);
        else
            m_bodySprite.setTexture(m_walkTextures[m_walkTexFrame], true);
    } else if (m_walkTexLoaded) {
        m_bodySprite.setTexture(m_walkTextures[0], true);
    }
}