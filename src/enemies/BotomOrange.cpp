#include "enemies/BotomOrange.hpp"
#include <cstdlib>
#include <cstdio>
#include <iostream>

namespace {
    const float BLUE_SPRITE_W = 44.f;
    const float BLUE_SPRITE_H = 42.f;
    const float BLUE_HIT_W    = 28.f;
    const float BLUE_HIT_H    = 34.f;

    // 125% of Botom red speed (80 * 1.25 = 100)
    const float BLUE_SPEED    = 100.f;

    // 3 hits required for full encasement
    const int   BLUE_HITS_TO_ENCASE = 3;

    const float SNOWBALLED_DURATION   = 3.0f;
    const float ESCAPE_STAGE_DURATION = 1.0f;

    const sf::Color FROSTY_TINT(180, 220, 255);
}

float BotomOrange::randomBetween(float lo, float hi) {
    float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return lo + t * (hi - lo);
}

BotomOrange::BotomOrange(sf::Vector2f pos)
    : Enemy(pos,
            BLUE_SPRITE_W, BLUE_SPRITE_H,
            BLUE_HIT_W,    BLUE_HIT_H,
            BLUE_SPEED,
            BLUE_HITS_TO_ENCASE)
    , m_encased25Loaded(false)
    , m_encased50Loaded(false)
    , m_directionTimer(0.f)
    , m_jumpTimer(0.f)
    , m_lastWalkVelocityX(0.f)
{
    std::cerr << "[BotomOrange] CREATED with hitsToEncase=" << m_hitsToEncase << "\n";
    // Load base enemy assets (idle, trapped, unleash frames, standard snow overlays)
    // Blue botom uses its own unleash frames; trapped frame reuses botom_idle as fallback.
    loadEnemyAssets(
        "assets/sprites/botom_idle.png",           // idle (shared with red)
        "assets/sprites/botom_trapped.png",         // fully snowballed body
        "assets/sprites/botom_orange_unleashed75.png",
        "assets/sprites/botom_orange_unleashed50.png",
        "assets/sprites/botom_orange_unleashed25.png",
        "assets/sprites/snow_encase_50.png",        // base 50% overlay (hit 1 — will be overridden)
        "assets/sprites/snow_encase_100.png",       // 100% overlay (fully snowballed)
        "assets/sprites/snow_escape_75.png",
        "assets/sprites/snow_escape_50.png",
        "assets/sprites/snow_escape_25.png"
    );

    // Load blue-specific encasement overlay textures
    auto tryLoad = [&](sf::Texture& tex, bool& flag, const char* path) {
        if (std::FILE* f = std::fopen(path, "rb")) {
            std::fclose(f);
            flag = tex.loadFromFile(path);
        } else {
            std::cerr << "[BotomOrange] missing: " << path << "\n";
            flag = false;
        }
    };
    tryLoad(m_encased25Texture, m_encased25Loaded, "assets/sprites/botom_orange_encased25.png");
    tryLoad(m_encased50Texture, m_encased50Loaded, "assets/sprites/botom_orange_encased50.png");

    // Walk animation — loadAnimations expects "basepath" + "1/2/3.png"
    // Pass a dummy jump path that contains "jumping" so the auto-derive doesn't
    // corrupt m_fallTexture; we immediately overwrite both below.
    loadAnimations(
        "assets/sprites/botom_orange_walk_frame",
        "assets/sprites/botom_red_jumping"   // temporary — overwritten below
    );

    // Override jump & fall with the correct blue-specific frames.
    // loadAnimations auto-derives fall by replacing "jumping"→"falling", which
    // would give botom_red_falling.png. We overwrite both explicitly here.
    m_jumpLoaded = false;
    m_fallLoaded = false;
    if (std::FILE* f = std::fopen("assets/sprites/botom_orange_jump.png", "rb")) {
        std::fclose(f);
        m_jumpLoaded = m_jumpTexture.loadFromFile("assets/sprites/botom_orange_jump.png");
    }
    if (!m_jumpLoaded)
        std::cerr << "[BotomOrange] missing: assets/sprites/botom_orange_jump.png\n";

    if (std::FILE* f = std::fopen("assets/sprites/botom_orange_fall.png", "rb")) {
        std::fclose(f);
        m_fallLoaded = m_fallTexture.loadFromFile("assets/sprites/botom_orange_fall.png");
    }
    if (!m_fallLoaded)
        std::cerr << "[BotomOrange] missing: assets/sprites/botom_orange_fall.png\n";

    // Enemy base constructor leaves m_oneHitEncase uninitialised — force false.
    setOneHitEncase(false);

    m_facingRight = (std::rand() % 2 == 0);
    rollDirectionTimer();
    rollJumpTimer();
}

void BotomOrange::rollDirectionTimer() {
    m_directionTimer = randomBetween(1.0f, 3.0f);
}

void BotomOrange::rollJumpTimer() {
    m_jumpTimer = randomBetween(3.0f, 6.0f);
}

// ------------------------------------------------------------------
// updateAI — identical logic to Botom (red), just faster speed
// ------------------------------------------------------------------
void BotomOrange::updateAI(float dt) {
    if (m_state != State::Alive) {
        velocity.x = 0.f;
        return;
    }

    m_directionTimer -= dt;
    if (m_directionTimer <= 0.f) {
        m_facingRight = !m_facingRight;
        rollDirectionTimer();
    }

    if (m_lastWalkVelocityX != 0.f && velocity.x == 0.f) {
        m_facingRight = !m_facingRight;
        rollDirectionTimer();
    }

    m_jumpTimer -= dt;
    if (m_jumpTimer <= 0.f) {
        if (m_onGround) {
            jump();
            rollJumpTimer();
        } else {
            m_jumpTimer = 0.f;
        }
    }

    velocity.x = m_facingRight ? m_speed : -m_speed;
    m_lastWalkVelocityX = velocity.x;
}

// ------------------------------------------------------------------
// updateStateTimers — overrides base to handle 3-hit partial decay.
// Instead of resetting m_hitsTaken to 0 on PartialEncase expiry,
// we decrement by 1 so the enemy gradually loses snow layers:
//   hits=2 → hits=1 (still PartialEncase, timer restarts)
//   hits=1 → hits=0, state=Alive
// All other states delegate to the base class.
// ------------------------------------------------------------------
void BotomOrange::updateStateTimers(float dt) {
    if (m_state == State::PartialEncase) {
        m_stateTimer -= dt;
        if (m_stateTimer <= 0.f) {
            --m_hitsTaken;
            if (m_hitsTaken <= 0) {
                m_hitsTaken = 0;
                m_state = State::Alive;
                m_stateTimer = 0.f;
            } else {
                // Still partially encased — restart timer for next decay stage
                m_stateTimer = getPartialEncaseDuration();
            }
        }
        return;
    }
    // All other states (Snowballed, Escaping75/50/25) handled by base
    Enemy::updateStateTimers(dt);
}

// ------------------------------------------------------------------
//   m_hitsTaken == 1 → botom_orange_encased25 overlay
//   m_hitsTaken == 2 → botom_orange_encased50 overlay
//   Snowballed       → snow_encase_100 (ready to roll)
// ------------------------------------------------------------------
void BotomOrange::applyStateSprite() {
    m_overlayVisible = false;
    m_bodySprite.setColor(sf::Color::White);

    auto setBody = [&](const sf::Texture& tex, bool loaded) {
        if (loaded) m_bodySprite.setTexture(tex, true);
    };
    auto setOverlay = [&](const sf::Texture& tex, bool loaded) {
        if (loaded) {
            m_overlaySprite.setTexture(tex, true);
            m_overlaySprite.setColor(sf::Color::White);
            m_overlayVisible = true;
        }
    };

    switch (m_state) {
        case State::Alive:
            // animation handles body texture
            break;

        case State::PartialEncase:
            setBody(m_trappedTexture, m_trappedLoaded);
            std::cerr << "[BotomOrange] PartialEncase hitsTaken=" << m_hitsTaken
                      << " enc25=" << m_encased25Loaded
                      << " enc50=" << m_encased50Loaded << "\n";
            if (m_hitsTaken == 1) {
                // First hit: 25% snow overlay
                if (m_encased25Loaded)
                    setOverlay(m_encased25Texture, true);
                else if (m_snowEncase50Loaded)
                    setOverlay(m_snowEncase50Texture, true);  // fallback
                else
                    m_bodySprite.setColor(sf::Color(210, 235, 255));
            } else {
                // Second hit: 50% snow overlay (botom_orange_encased_50)
                if (m_encased50Loaded)
                    setOverlay(m_encased50Texture, true);
                else if (m_snowEncase50Loaded)
                    setOverlay(m_snowEncase50Texture, true);  // fallback
                else
                    m_bodySprite.setColor(FROSTY_TINT);
            }
            break;

        case State::Snowballed:
            setBody(m_trappedTexture, m_trappedLoaded);
            // 3rd hit = fully snowballed, ready to roll — use standard snow_encase_100
            if (m_snowEncase100Loaded)
                setOverlay(m_snowEncase100Texture, true);
            else
                m_bodySprite.setColor(FROSTY_TINT);
            break;

        case State::Escaping75:
            setBody(m_unleash1Texture, m_unleash1Loaded);
            if (m_snowEscape75Loaded) setOverlay(m_snowEscape75Texture, true);
            else m_bodySprite.setColor(FROSTY_TINT);
            break;

        case State::Escaping50:
            setBody(m_unleash2Texture, m_unleash2Loaded);
            if (m_snowEscape50Loaded) setOverlay(m_snowEscape50Texture, true);
            else m_bodySprite.setColor(sf::Color(210, 230, 250));
            break;

        case State::Escaping25:
            setBody(m_unleash3Texture, m_unleash3Loaded);
            if (m_snowEscape25Loaded) setOverlay(m_snowEscape25Texture, true);
            else m_bodySprite.setColor(sf::Color(235, 245, 255));
            break;

        case State::Rolling:
            setBody(m_trappedTexture, m_trappedLoaded);
            if (m_snowEncase100Loaded)
                setOverlay(m_snowEncase100Texture, true);
            break;

        case State::Dead:
            break;
    }
}