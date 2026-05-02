#include "enemies/FlyngFoogaFoog.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include <iostream>
#include <cmath>

extern Player*    g_player;
extern Platform** g_platforms;
extern int        g_platformCount;

static const float GRAVITY = 800.f;

FlyngFoogaFoog::FlyngFoogaFoog(sf::Vector2f pos)
    : Enemy(pos, 56.f, 68.f, 40.f, 50.f, 80.f, 4)
    , m_isFlying(false)
    , m_flyTimer(0.f)
    , m_walkTimer(0.f)   // starts in walk mode
    , m_flyLoaded(false)
    , m_flyFrame(0)
    , m_flyFrameTimer(0.f)
    , m_fooga25Loaded(false)
    , m_fooga50Loaded(false)
    , m_fooga75Loaded(false)
{
    // Idle + snow100 (for Snowballed/Rolling) use existing assets
    loadEnemyAssets(
        "assets/sprites/fooga_idle_blue.png",
        "assets/sprites/fooga_idle_blue.png",   // trapped = idle frame
        "assets/sprites/fooga_idle_blue.png",   // unleash1 = idle
        "assets/sprites/fooga_idle_blue.png",   // unleash2 = idle
        "assets/sprites/fooga_idle_blue.png",   // unleash3 = idle
        "assets/sprites/fooga_blue_50.png",     // encase50  (unused directly)
        "assets/sprites/snow_encase_100.png",   // full snowball for rolling
        "assets/sprites/snow_escape_75.png",    // escape overlays (unused)
        "assets/sprites/snow_escape_50.png",
        "assets/sprites/snow_escape_25.png"
    );

    // Fooga-specific partial encasement textures (no overlay — full replacement)
    m_fooga25Loaded = m_fooga25Texture.loadFromFile("assets/sprites/fooga_blue_25.png");
    m_fooga50Loaded = m_fooga50Texture.loadFromFile("assets/sprites/fooga_blue_50.png");
    m_fooga75Loaded = m_fooga75Texture.loadFromFile("assets/sprites/fooga_blue_75.png");

    // Flying animation
    m_flyLoaded = true;
    for (int i = 0; i < 3; ++i) {
        std::string path = "assets/sprites/fooga_fly_blue_frame"
                         + std::to_string(i + 1) + ".png";
        if (!m_flyTextures[i].loadFromFile(path)) {
            std::cerr << "[FlyngFoogaFoog] Missing: " << path << "\n";
            m_flyLoaded = false;
        }
    }
}

// ---------------------------------------------------------------
// update — override to skip gravity when flying
// ---------------------------------------------------------------
void FlyngFoogaFoog::update(float dt) {
    if (m_state == State::Rolling) {
        updateStateTimers(dt);
        integrateRolling(dt);
        syncHitBox();
        applyStateSprite();
        syncSpritePositions();
        return;
    }

    updateStateTimers(dt);

    if (m_state == State::Alive) {
        // Only apply gravity in walk mode
        if (!m_isFlying) {
            velocity.y += GRAVITY * dt;
        }
        updateAI(dt);
    } else {
        // Encased/escaping: gravity always, stop horizontal
        velocity.y += GRAVITY * dt;
        velocity.x = 0.f;
        // Force to idle frame when hit while flying
        m_isFlying = false;
    }

    position += velocity * dt;
    syncHitBox();
    applyStateSprite();
    updateAnimation(dt);
    syncSpritePositions();
}

// ---------------------------------------------------------------
// updateAI — fly 5s chasing player, walk 8s on platforms
// ---------------------------------------------------------------
void FlyngFoogaFoog::updateAI(float dt) {
    if (m_isFlying) {
        m_flyTimer += dt;
        if (m_flyTimer >= FLY_DURATION) {
            m_isFlying   = false;
            m_walkTimer  = 0.f;
            m_flyTimer   = 0.f;
            velocity.y   = 0.f;   // stop vertical on landing
        }
    } else {
        m_walkTimer += dt;
        if (m_walkTimer >= WALK_DURATION) {
            m_isFlying  = true;
            m_flyTimer  = 0.f;
            m_walkTimer = 0.f;
            velocity.y  = 0.f;
        }
    }

    if (m_isFlying) {
        // Chase player — ignores platforms, moves freely in 2D
        if (g_player) {
            sf::Vector2f pPos = g_player->getPosition();
            sf::Vector2f dir  = pPos - position;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 4.f) {
                dir.x /= len;
                dir.y /= len;
                velocity.x = dir.x * 100.f;
                velocity.y = dir.y * 100.f;
                m_facingRight = (dir.x > 0.f);
            } else {
                velocity.x = 0.f;
                velocity.y = 0.f;
            }
        }
    } else {
        // Walk mode — same as Botom
        if (!m_onGround) return;

        if (velocity.x == 0.f)
            velocity.x = m_facingRight ? m_speed : -m_speed;

        // Edge detection: check if ground exists ahead
        sf::FloatRect ahead = hitBox;
        ahead.position.x += m_facingRight ? hitBox.size.x : -20.f;
        ahead.size.x = 20.f;

        bool hasGround = false;
        for (int i = 0; i < g_platformCount && !hasGround; ++i) {
            if (!g_platforms[i]) continue;
            for (int b = 0; b < g_platforms[i]->getHitboxCount(); ++b) {
                sf::FloatRect pb = g_platforms[i]->getBounds(b);
                bool xOverlap = ahead.position.x + ahead.size.x > pb.position.x
                             && ahead.position.x < pb.position.x + pb.size.x;
                bool yEdge    = ahead.position.y + ahead.size.y >= pb.position.y - 10.f
                             && ahead.position.y + ahead.size.y <= pb.position.y + 10.f;
                if (xOverlap && yEdge) { hasGround = true; break; }
            }
        }

        if (!hasGround) {
            m_facingRight = !m_facingRight;
            velocity.x    = -velocity.x;
        }

        // Occasional random jump (same as Botom)
        if (std::rand() % 200 == 0) jump();
    }
}

// ---------------------------------------------------------------
// applyStateSprite — fooga-specific: no botom overlay, use own textures
// ---------------------------------------------------------------
void FlyngFoogaFoog::applyStateSprite() {
    m_overlayVisible = false;
    m_bodySprite.setColor(sf::Color::White);

    switch (m_state) {
        case State::Alive:
            // animation handles this
            break;

        case State::PartialEncase:
            // Show correct % texture based on hits taken
            if (m_hitsTaken == 1 && m_fooga25Loaded)
                m_bodySprite.setTexture(m_fooga25Texture, true);
            else if (m_hitsTaken == 2 && m_fooga50Loaded)
                m_bodySprite.setTexture(m_fooga50Texture, true);
            else if (m_hitsTaken == 3 && m_fooga75Loaded)
                m_bodySprite.setTexture(m_fooga75Texture, true);
            break;

        case State::Snowballed:
            if (m_idleLoaded) m_bodySprite.setTexture(m_idleTexture, true);
            if (m_snowEncase100Loaded) {
                m_overlaySprite.setTexture(m_snowEncase100Texture, true);
                m_overlaySprite.setColor(sf::Color::White);
                m_overlayVisible = true;
            }
            break;

        // Escape stages — reverse: 75→50→25→free
        case State::Escaping75:
            if (m_fooga75Loaded) m_bodySprite.setTexture(m_fooga75Texture, true);
            break;
        case State::Escaping50:
            if (m_fooga50Loaded) m_bodySprite.setTexture(m_fooga50Texture, true);
            break;
        case State::Escaping25:
            if (m_fooga25Loaded) m_bodySprite.setTexture(m_fooga25Texture, true);
            break;

        case State::Rolling:
            if (m_idleLoaded) m_bodySprite.setTexture(m_idleTexture, true);
            if (m_snowEncase100Loaded) {
                m_overlaySprite.setTexture(m_snowEncase100Texture, true);
                m_overlaySprite.setColor(sf::Color::White);
                m_overlayVisible = true;
            }
            break;

        case State::Dead:
            break;
    }
}

// ---------------------------------------------------------------
// updateAnimation — fly frames while flying, idle while walking
// ---------------------------------------------------------------
void FlyngFoogaFoog::updateAnimation(float dt) {
    if (m_state != State::Alive) return;

    if (m_isFlying && m_flyLoaded) {
        m_flyFrameTimer += dt;
        if (m_flyFrameTimer >= FLY_FRAME_TIME) {
            m_flyFrameTimer -= FLY_FRAME_TIME;
            m_flyFrame = (m_flyFrame + 1) % 3;
        }
        m_bodySprite.setTexture(m_flyTextures[m_flyFrame], true);
    } else {
        if (m_idleLoaded)
            m_bodySprite.setTexture(m_idleTexture, true);
        m_flyFrame      = 0;
        m_flyFrameTimer = 0.f;
    }
}