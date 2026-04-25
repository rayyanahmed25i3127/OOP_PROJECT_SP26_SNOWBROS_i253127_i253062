#include "enemies/Enemy.hpp"
#include <cstdio>
#include <iostream>

namespace {
    // Lifecycle timings
    const float SNOWBALLED_DURATION   = 3.0f;   // before shake-free chain begins
    const float ESCAPE_STAGE_DURATION = 1.0f;   // each of 75 / 50 / 25

    // Rolling physics
    const float ROLL_SPEED = 650.f;             // 600-700 range (design choice)
    const float WINDOW_WIDTH = 800.f;
    const float WINDOW_LEFT  = 30.f;            // matches CollisionDetector
    const float WINDOW_RIGHT = 770.f;

    // Fallback visual tint when a snow overlay asset is missing.
    const sf::Color FROSTY_TINT(180, 220, 255);
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
        bool flip = !m_facingRight
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
                // Partial coat shakes off — back to Alive with no hits.
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
            setBody(m_idleTexture, m_idleLoaded);
            break;
        case State::PartialEncase:
            // Body = trapped pose; overlay = 50% snow.
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
            // Rolling enemy looks like a Snowballed one, but moving.
            setBody(m_trappedTexture, m_trappedLoaded);
            if (m_snowEncase100Loaded) setOverlay(m_snowEncase100Texture, true);
            break;
        case State::Dead:
            break;
    }
}

void Enemy::integrateRolling(float dt) {
    // Rolling enemies ignore gravity (keep y constant — arcade-style behavior)
    // and travel horizontally at ROLL_SPEED. Wrap screen once. Die when we
    // return to kick-origin x.
    float dx = (m_rollDirectionRight ? 1.f : -1.f) * m_rollSpeed * dt;
    position.x += dx;
    m_rollDistanceTravelled += (dx < 0 ? -dx : dx);
    velocity.x = 0.f;
    velocity.y = 0.f;   // stay aloft (simple arcade rule)

    // Wrap edges
    float hitW = hitBox.size.x;
    if (position.x + hitW < WINDOW_LEFT) {
        position.x = WINDOW_RIGHT;   // enter from right
    } else if (position.x > WINDOW_RIGHT) {
        position.x = WINDOW_LEFT - hitW;  // enter from left
    }

    // Die when we've travelled roughly one full screen width and we're
    // back near the kick origin. Use distance threshold (WINDOW_WIDTH)
    // so we don't die on the first frame by accident.
    if (m_rollDistanceTravelled >= WINDOW_RIGHT - WINDOW_LEFT) {
        // Check if position is near origin (within half a sprite)
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
        // Rolling state has its own motion integration — skip gravity + AI.
        updateStateTimers(dt);       // no-op for Rolling, but kept for consistency
        integrateRolling(dt);
        syncHitBox();
        applyStateSprite();
        syncSpritePositions();
        return;
    }

    applyGravity(dt);
    updateStateTimers(dt);

    // Only Alive enemies run AI. Frozen states keep velocity zeroed.
    if (m_state == State::Alive) {
        updateAI(dt);
    } else {
        velocity.x = 0.f;
        // gravity already applied — still let them fall if mid-air
    }

    position += velocity * dt;
    syncHitBox();
    applyStateSprite();
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
    // Accept hits in Alive, PartialEncase, and any Escaping stage.
    // Snowballed / Rolling / Dead ignore further hits.
    if (m_state != State::Alive &&
        m_state != State::PartialEncase &&
        m_state != State::Escaping75 &&
        m_state != State::Escaping50 &&
        m_state != State::Escaping25)
    {
        return;
    }

    // Escaping75 / Escaping50 — lots of snow still on the enemy.
    // One hit packs it back into a full Snowball (resets the 3s timer).
    if (m_state == State::Escaping75 || m_state == State::Escaping50) {
        m_state = State::Snowballed;
        m_stateTimer = SNOWBALLED_DURATION;
        m_hitsTaken = m_hitsToEncase;
        velocity = { 0.f, 0.f };
        return;
    }

    // Escaping25 — barely any snow left. Fresh encase cycle needed:
    // first hit -> PartialEncase, second hit -> Snowballed.
    if (m_state == State::Escaping25) {
        m_state = State::PartialEncase;
        m_hitsTaken = 1;
        m_stateTimer = getPartialEncaseDuration();
        velocity = { 0.f, 0.f };
        return;
    }

    // --- Standard path: Alive or PartialEncase ---
    ++m_hitsTaken;
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
    // velocity set to zero — integrateRolling handles motion via m_rollSpeed
    velocity = { 0.f, 0.f };
}