#include "enemies/Enemy.hpp"
#include <cstdio>
#include <iostream>

namespace {
    const float SNOWBALLED_DURATION   = 3.0f;
    const float ESCAPE_STAGE_DURATION = 1.0f;

    const float ROLL_SPEED   = 650.f;
    const float WINDOW_WIDTH = 800.f;
    const float WINDOW_LEFT  = 30.f;
    const float WINDOW_RIGHT = 770.f;

    const sf::Color FROSTY_TINT(180, 220, 255);

    // Walk animation timing (same as player: 0.33s per frame, 1s full cycle)
    const float WALK_FRAME_TIME = 0.33f;
}

bool Enemy::tryLoadTexture(sf::Texture& out, const std::string& path,
                           const char* tag) {
    if (std::FILE* f = std::fopen(path.c_str(), "rb")) {
        std::fclose(f);
        if (out.loadFromFile(path)) return true;
        std::cerr << "[Enemy] " << tag << " decode fail: " << path << "\n";
        return false;
    }
    std::cerr << "[Enemy] " << tag << " missing: " << path << "\n";
    return false;
}

void Enemy::loadEnemyAssets(
    const std::string& idlePath,
    const std::string& trappedPath,
    const std::string& unleash1Path,
    const std::string& unleash2Path,
    const std::string& unleash3Path,
    const std::string& snowEncase50Path,
    const std::string& snowEncase100Path,
    const std::string& snowEscape75Path,
    const std::string& snowEscape50Path,
    const std::string& snowEscape25Path
) {
    m_idleLoaded     = tryLoadTexture(m_idleTexture,     idlePath,     "idle");
    m_trappedLoaded  = tryLoadTexture(m_trappedTexture,  trappedPath,  "trapped");
    m_unleash1Loaded = tryLoadTexture(m_unleash1Texture, unleash1Path, "unleash1");
    m_unleash2Loaded = tryLoadTexture(m_unleash2Texture, unleash2Path, "unleash2");
    m_unleash3Loaded = tryLoadTexture(m_unleash3Texture, unleash3Path, "unleash3");

    m_snowEncase50Loaded  = tryLoadTexture(m_snowEncase50Texture,  snowEncase50Path,  "snow50");
    m_snowEncase100Loaded = tryLoadTexture(m_snowEncase100Texture, snowEncase100Path, "snow100");
    m_snowEscape75Loaded  = tryLoadTexture(m_snowEscape75Texture,  snowEscape75Path,  "escape75");
    m_snowEscape50Loaded  = tryLoadTexture(m_snowEscape50Texture,  snowEscape50Path,  "escape50");
    m_snowEscape25Loaded  = tryLoadTexture(m_snowEscape25Texture,  snowEscape25Path,  "escape25");

    if (m_idleLoaded) m_bodySprite.setTexture(m_idleTexture, true);
}

Enemy::Enemy(sf::Vector2f pos,
             float spriteW, float spriteH,
             float hitW,    float hitH,
             float speed,
             int   hitsToEncase)
    : Entity(pos)
    , m_idleLoaded(false)
    , m_trappedLoaded(false)
    , m_unleash1Loaded(false)
    , m_unleash2Loaded(false)
    , m_unleash3Loaded(false)
    , m_bodySprite(m_idleTexture)
    , m_snowEncase50Loaded(false)
    , m_snowEncase100Loaded(false)
    , m_snowEscape75Loaded(false)
    , m_snowEscape50Loaded(false)
    , m_snowEscape25Loaded(false)
    , m_overlaySprite(m_snowEncase100Texture)
    , m_overlayVisible(false)
    , m_walkLoaded(false)
    , m_jumpLoaded(false)
    , m_fallLoaded(false)
    , m_walkFrame(0)
    , m_walkTimer(0.f)
    , m_spriteWidth(spriteW)
    , m_spriteHeight(spriteH)
    , m_overlayWidth(spriteW + 4.f)
    , m_overlayHeight(spriteH + 4.f)
    , m_hitOffsetX((spriteW - hitW) / 2.f)
    , m_hitOffsetY(spriteH - hitH)
    , m_speed(speed)
    , m_gravity(800.f)
    , m_jumpForce(-430.f)
    , m_facingRight(true)
    , m_onGround(false)
    , m_state(State::Alive)
    , m_hitsToEncase(hitsToEncase)
    , m_hitsTaken(0)
    , m_stateTimer(0.f)
    , m_rollOriginX(0.f)
    , m_rollDirectionRight(true)
    , m_rollDistanceTravelled(0.f)
    , m_rollSpeed(ROLL_SPEED)
    , m_oneHitEncase(false)
{
    hitBox.size     = { hitW, hitH };
    hitBox.position = { pos.x + m_hitOffsetX, pos.y + m_hitOffsetY };
}

void Enemy::applyGravity(float dt) {
    velocity.y += m_gravity * dt;
}

void Enemy::syncHitBox() {
    hitBox.position = { position.x + m_hitOffsetX,
                        position.y + m_hitOffsetY };
}

void Enemy::syncSpritePositions() {
    auto bodyTex = m_bodySprite.getTexture();
    sf::Vector2u bodySize = bodyTex.getSize();
    if (bodySize.x > 0 && bodySize.y > 0) {
        float sx = m_spriteWidth  / static_cast<float>(bodySize.x);
        float sy = m_spriteHeight / static_cast<float>(bodySize.y);
        m_bodySprite.setOrigin({ static_cast<float>(bodySize.x) / 2.f, 0.f });

        // Default sprites face LEFT. Flip when facing RIGHT and alive/rolling.
        bool flip = m_facingRight
                 && (m_state == State::Alive || m_state == State::Rolling);
        if (flip) m_bodySprite.setScale({ -sx, sy });
        else      m_bodySprite.setScale({  sx, sy });

        m_bodySprite.setPosition({ position.x + m_spriteWidth / 2.f,
                                   position.y });
    }
    if (m_overlayVisible) {
        auto ovTex = m_overlaySprite.getTexture();
        sf::Vector2u ovSize = ovTex.getSize();
        if (ovSize.x > 0 && ovSize.y > 0) {
            float sx = m_overlayWidth  / static_cast<float>(ovSize.x);
            float sy = m_overlayHeight / static_cast<float>(ovSize.y);
            m_overlaySprite.setOrigin({ static_cast<float>(ovSize.x) / 2.f, 0.f });
            m_overlaySprite.setScale({ sx, sy });
            m_overlaySprite.setPosition({ position.x + m_spriteWidth / 2.f,
                                          position.y - 2.f });
        }
    }
}

void Enemy::jump() {
    if (!m_onGround) return;
    velocity.y = m_jumpForce;
    m_onGround = false;
}

void Enemy::updateStateTimers(float dt) {
    switch (m_state) {
        case State::PartialEncase:
            m_stateTimer -= dt;
            if (m_stateTimer <= 0.f) {
                m_state = State::Alive;
                m_hitsTaken = 0;
                m_stateTimer = 0.f;
            }
            break;
        case State::Snowballed:
            m_stateTimer -= dt;
            if (m_stateTimer <= 0.f) {
                m_state = State::Escaping75;
                m_stateTimer = ESCAPE_STAGE_DURATION;
            }
            break;
        case State::Escaping75:
            m_stateTimer -= dt;
            if (m_stateTimer <= 0.f) {
                m_state = State::Escaping50;
                m_stateTimer = ESCAPE_STAGE_DURATION;
            }
            break;
        case State::Escaping50:
            m_stateTimer -= dt;
            if (m_stateTimer <= 0.f) {
                m_state = State::Escaping25;
                m_stateTimer = ESCAPE_STAGE_DURATION;
            }
            break;
        case State::Escaping25:
            m_stateTimer -= dt;
            if (m_stateTimer <= 0.f) {
                m_state = State::Alive;
                m_hitsTaken = 0;
                m_stateTimer = 0.f;
            }
            break;
        default:
            break;
    }
}

void Enemy::applyStateSprite() {
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
            // Animation handles texture for Alive state — don't override here
            break;
        case State::PartialEncase:
            setBody(m_trappedTexture, m_trappedLoaded);
            if (m_snowEncase50Loaded) setOverlay(m_snowEncase50Texture, true);
            else m_bodySprite.setColor(FROSTY_TINT);
            break;
        case State::Snowballed:
            setBody(m_trappedTexture, m_trappedLoaded);
            if (m_snowEncase100Loaded) setOverlay(m_snowEncase100Texture, true);
            else m_bodySprite.setColor(FROSTY_TINT);
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
            if (m_snowEncase100Loaded) setOverlay(m_snowEncase100Texture, true);
            break;
        case State::Dead:
            break;
    }
}

void Enemy::integrateRolling(float dt) {
    float dx = (m_rollDirectionRight ? 1.f : -1.f) * m_rollSpeed * dt;
    position.x += dx;
    m_rollDistanceTravelled += (dx < 0 ? -dx : dx);
    velocity.x = 0.f;
    velocity.y = 0.f;

    float hitW = hitBox.size.x;
    if (position.x + hitW < WINDOW_LEFT) {
        position.x = WINDOW_RIGHT;
    } else if (position.x > WINDOW_RIGHT) {
        position.x = WINDOW_LEFT - hitW;
    }

    if (m_rollDistanceTravelled >= WINDOW_RIGHT - WINDOW_LEFT) {
        float dx_origin = position.x - m_rollOriginX;
        if (dx_origin < 0) dx_origin = -dx_origin;
        if (dx_origin < m_spriteWidth * 0.5f) {
            m_state = State::Dead;
            alive = false;
        }
    }
}

void Enemy::update(float dt) {
    if (m_state == State::Rolling) {
        updateStateTimers(dt);
        integrateRolling(dt);
        syncHitBox();
        applyStateSprite();
        syncSpritePositions();
        return;
    }

    applyGravity(dt);
    updateStateTimers(dt);

    if (m_state == State::Alive) {
        updateAI(dt);
    } else {
        velocity.x = 0.f;
    }

    position += velocity * dt;
    syncHitBox();

    // Always call applyStateSprite — it resets m_overlayVisible to false
    // at the top, so when enemy returns to Alive the snow overlay disappears.
    // For Alive state it won't override the body texture (animation handles that).
    applyStateSprite();

    // For Alive state, animation swaps the body texture directly
    updateAnimation(dt);

    syncSpritePositions();
}

void Enemy::draw(sf::RenderWindow& window) {
    window.draw(m_bodySprite);
    if (m_overlayVisible) window.draw(m_overlaySprite);
}

void Enemy::setPosition(sf::Vector2f pos) {
    position = pos;
    syncHitBox();
    syncSpritePositions();
}

void Enemy::takeAttackHit() {
    if (m_state != State::Alive &&
        m_state != State::PartialEncase &&
        m_state != State::Escaping75 &&
        m_state != State::Escaping50 &&
        m_state != State::Escaping25)
    {
        return;
    }

    if (m_state == State::Escaping75 || m_state == State::Escaping50) {
        m_state = State::Snowballed;
        m_stateTimer = SNOWBALLED_DURATION;
        m_hitsTaken = m_hitsToEncase;
        velocity = { 0.f, 0.f };
        return;
    }

    if (m_state == State::Escaping25) {
        m_state = State::PartialEncase;
        m_hitsTaken = 1;
        m_stateTimer = getPartialEncaseDuration();
        velocity = { 0.f, 0.f };
        return;
    }

    ++m_hitsTaken;
    std::cerr << "[Enemy::takeAttackHit] hitsToEncase=" << m_hitsToEncase
              << " hitsTaken=" << m_hitsTaken
              << " oneHitEncase=" << m_oneHitEncase << "\n";
    if (m_hitsTaken >= m_hitsToEncase) {
        m_state = State::Snowballed;
        m_stateTimer = SNOWBALLED_DURATION;
        velocity = { 0.f, 0.f };
    } else {
        m_state = State::PartialEncase;
        m_stateTimer = getPartialEncaseDuration();
        velocity = { 0.f, 0.f };
    }
}

void Enemy::kickIntoRoll(bool facingRight) {
    if (m_state != State::Snowballed) return;

    m_state = State::Rolling;
    m_rollOriginX = position.x;
    m_rollDirectionRight = facingRight;
    m_rollDistanceTravelled = 0.f;
    m_facingRight = facingRight;
    velocity = { 0.f, 0.f };
}

// ---------------------------------------------------------------
// loadAnimations — loads walk/jump/fall textures into PERSISTENT
// class member arrays. These textures live as long as the enemy.
// ---------------------------------------------------------------
void Enemy::loadAnimations(const std::string& walkBasePath, const std::string& jumpBasePath) {
    // walkBasePath = "assets/sprites/botom_red_walking_frame"  (no number, no .png)
    // jumpBasePath = "assets/sprites/botom_red_jumping"        (no .png)

    // === WALK (3 frames — stored in m_walkTextures[]) ===
    m_walkLoaded = true;
    for (int i = 0; i < 3; ++i) {
        std::string path = walkBasePath + std::to_string(i + 1) + ".png";
        if (!m_walkTextures[i].loadFromFile(path)) {
            std::cerr << "[Enemy] Failed to load " << path << "\n";
            m_walkLoaded = false;
            break;
        }
    }

    // === JUMP (1 frame) ===
    std::string jumpPath = jumpBasePath + ".png";
    m_jumpLoaded = m_jumpTexture.loadFromFile(jumpPath);
    if (!m_jumpLoaded)
        std::cerr << "[Enemy] Failed to load " << jumpPath << "\n";

    // === FALL (1 frame — derive path by replacing "jumping" with "falling") ===
    std::string fallPath = jumpBasePath;
    size_t pos = fallPath.find("jumping");
    if (pos != std::string::npos) {
        fallPath.replace(pos, 7, "falling");
    }
    fallPath += ".png";
    m_fallLoaded = m_fallTexture.loadFromFile(fallPath);
    if (!m_fallLoaded)
        std::cerr << "[Enemy] Failed to load " << fallPath << "\n";
}

// ---------------------------------------------------------------
// updateAnimation — direct texture swap based on enemy state.
// Only runs for Alive enemies. Frozen states use applyStateSprite().
// ---------------------------------------------------------------
void Enemy::updateAnimation(float dt) {
    if (m_state != State::Alive) return;

    // --- AIRBORNE ---
    if (!m_onGround) {
        if (velocity.y < 0.f && m_jumpLoaded) {
            // Going UP → jump frame
            m_bodySprite.setTexture(m_jumpTexture, true);
        } else if (m_fallLoaded) {
            // Going DOWN → fall frame
            m_bodySprite.setTexture(m_fallTexture, true);
        } else if (m_jumpLoaded) {
            // Fallback: use jump frame for fall too
            m_bodySprite.setTexture(m_jumpTexture, true);
        }
        // Reset walk so it starts clean on landing
        m_walkFrame = 0;
        m_walkTimer = 0.f;
        return;
    }

    // --- WALKING (on ground, moving) ---
    if (velocity.x != 0.f && m_walkLoaded) {
        m_walkTimer += dt;
        if (m_walkTimer >= WALK_FRAME_TIME) {
            m_walkTimer -= WALK_FRAME_TIME;
            m_walkFrame = (m_walkFrame + 1) % 3;
        }
        m_bodySprite.setTexture(m_walkTextures[m_walkFrame], true);
        return;
    }

    // --- IDLE (on ground, not moving) ---
    if (m_idleLoaded) {
        m_bodySprite.setTexture(m_idleTexture, true);
    }
    m_walkFrame = 0;
    m_walkTimer = 0.f;
}