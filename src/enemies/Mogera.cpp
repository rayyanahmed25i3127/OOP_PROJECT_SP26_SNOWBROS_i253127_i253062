#include "enemies/Mogera.hpp"
#include <iostream>
#include <cstdio>

namespace {
    // Lower-right platform: sprite pos y=437, hitbox insetTop=20 → surface at y=457.
    // Mogera's feet must sit at y=457, so position.y = 457 - spriteH.
    const float PLATFORM_TOP_Y = 475.f;   // actual landing surface of lower platforms
}

Mogera::Mogera(sf::Vector2f pos)
    : Entity(pos)
    , m_sprite(m_idleTexture)          // SFML3: must pass texture to constructor
    , m_idleLoaded(false)
    , m_open1Loaded(false)
    , m_open2Loaded(false)
    , m_deathLoaded(false)
    , m_bossState(BossState::Idle)
    , m_stateTimer(IDLE_INTRO)         // start with a brief idle before first attack
    , m_maxHits(50)
    , m_hitsRemaining(50)
    , m_canAttack(true)
    , m_babiesSpawnedThisCycle(false)
    , m_spriteW(170.f)
    , m_spriteH(200.f)
    , m_rewardPending(false)
{
    m_spawnRequest.pending = false;

    // --- Load textures with fopen guard (SFML3 Windows compat) ---
    auto tryLoad = [&](sf::Texture& tex, const char* path, bool& flag, const char* tag) {
        if (std::FILE* f = std::fopen(path, "rb")) {
            std::fclose(f);
            if (tex.loadFromFile(path)) {
                flag = true;
            } else {
                std::cerr << "[Mogera] decode fail: " << tag << "\n";
            }
        } else {
            std::cerr << "[Mogera] missing: " << tag << " (" << path << ")\n";
        }
    };

    tryLoad(m_idleTexture,      "assets/sprites/mogera_idle.png",             m_idleLoaded,  "idle");
    tryLoad(m_openMouth1Texture,"assets/sprites/mogera_open_mouth_frame1.png",m_open1Loaded, "open1");
    tryLoad(m_openMouth2Texture,"assets/sprites/mogera_open_mouth_frame2.png",m_open2Loaded, "open2");
    tryLoad(m_deathTexture,     "assets/sprites/mogera_death.png",            m_deathLoaded, "death");

    if (m_idleLoaded) {
        m_sprite.setTexture(m_idleTexture, true);
    }

    // --- Hitbox: slightly smaller than sprite ---
    hitBox.size     = { m_spriteW - 20.f, m_spriteH - 10.f };
    hitBox.position = { pos.x + 10.f,     pos.y + 5.f };

    syncSprite();
}

// ---------------------------------------------------------------
void Mogera::syncHitBox() {
    hitBox.position = { position.x + 10.f, position.y + 5.f };
}

void Mogera::syncSprite() {
    sf::Texture* tex = nullptr;

    switch (m_bossState) {
        case BossState::Idle:
            if (m_idleLoaded) tex = &m_idleTexture;
            break;
        case BossState::OpenMouth1:
            tex = m_open1Loaded ? &m_openMouth1Texture
                                : (m_idleLoaded ? &m_idleTexture : nullptr);
            break;
        case BossState::OpenMouth2:
            tex = m_open2Loaded ? &m_openMouth2Texture
                                : (m_idleLoaded ? &m_idleTexture : nullptr);
            break;
        case BossState::Dying:
        case BossState::Dead:
            if (m_deathLoaded) tex = &m_deathTexture;
            else if (m_idleLoaded) tex = &m_idleTexture;
            break;
    }

    if (tex) {
        auto ts = tex->getSize();
        if (ts.x > 0 && ts.y > 0) {
            // Mogera faces LEFT — sprites drawn facing left by default, positive scale
            float sx = m_spriteW / static_cast<float>(ts.x);
            float sy = m_spriteH / static_cast<float>(ts.y);
            m_sprite.setTexture(*tex, true);
            m_sprite.setScale({ sx, sy });
            m_sprite.setPosition(position);
        }
    }
}

// ---------------------------------------------------------------
void Mogera::update(float dt) {
    if (m_bossState == BossState::Dead) return;

    m_stateTimer -= dt;

    if (m_bossState == BossState::Dying) {
        // Fall until feet touch the ground (PLATFORM_TOP_Y = actual platform surface)
        float groundY = PLATFORM_TOP_Y - m_spriteH + 50.f;   // top of sprite when feet on surface
        if (position.y < groundY) {
            velocity.y += DEATH_GRAVITY * dt;
            position   += velocity * dt;
            if (position.y >= groundY) {
                position.y = groundY;
                velocity   = { 0.f, 0.f };
            }
        }

        syncHitBox();
        syncSprite();

        if (m_stateTimer <= 0.f) {
            m_bossState = BossState::Dead;
            alive       = false;
            std::cout << "[Mogera] Death animation complete — removed\n";
        }
        return;
    }

    // ---- Normal attack cycle ----
    switch (m_bossState) {

        case BossState::Idle:
            if (m_stateTimer <= 0.f) {
                // Transition: Idle → OpenMouth1
                m_bossState = BossState::OpenMouth1;
                m_stateTimer = OPEN1_DURATION;
                syncSprite();
                std::cout << "[Mogera] Opening mouth (frame 1)\n";
            }
            break;

        case BossState::OpenMouth1:
            if (m_stateTimer <= 0.f) {
                // Transition: OpenMouth1 → OpenMouth2
                m_bossState = BossState::OpenMouth2;
                m_stateTimer = OPEN2_DURATION;
                m_babiesSpawnedThisCycle = false;
                syncSprite();
                std::cout << "[Mogera] Opening mouth (frame 2) — ready to throw\n";
            }
            break;

        case BossState::OpenMouth2:
            // Throw babies exactly once at the start of this frame
            if (!m_babiesSpawnedThisCycle && m_canAttack) {
                m_babiesSpawnedThisCycle = true;
                // Signal PlayState to spawn 3 babies
                // Mouth position: left edge of Mogera's sprite + a bit up from center
                m_spawnRequest.pending  = true;
                m_spawnRequest.spawnPos = {
                    position.x,                      // left side of Mogera (mouth opens left)
                    position.y + m_spriteH * 0.35f   // ~35% down from top = mouth height
                };
                std::cout << "[Mogera] Spawn request sent (3 babies)\n";
            }

            if (m_stateTimer <= 0.f) {
                // Transition: OpenMouth2 → Idle (long wait)
                m_bossState  = BossState::Idle;
                m_stateTimer = IDLE_BEFORE_ATTACK;
                syncSprite();
                std::cout << "[Mogera] Mouth closed — idle " << IDLE_BEFORE_ATTACK << "s\n";
            }
            break;

        default: break;
    }
}

// ---------------------------------------------------------------
void Mogera::draw(sf::RenderWindow& window) {
    if (!alive && m_bossState != BossState::Dying) return;
    if (m_bossState == BossState::Dead) return;

    sf::Texture* tex = nullptr;
    switch (m_bossState) {
        case BossState::Idle:       tex = m_idleLoaded  ? &m_idleTexture       : nullptr; break;
        case BossState::OpenMouth1: tex = m_open1Loaded ? &m_openMouth1Texture : (m_idleLoaded ? &m_idleTexture : nullptr); break;
        case BossState::OpenMouth2: tex = m_open2Loaded ? &m_openMouth2Texture : (m_idleLoaded ? &m_idleTexture : nullptr); break;
        case BossState::Dying:
        case BossState::Dead:       tex = m_deathLoaded ? &m_deathTexture      : (m_idleLoaded ? &m_idleTexture : nullptr); break;
    }

    if (tex) {
        auto ts = tex->getSize();
        if (ts.x > 0 && ts.y > 0) {
            float sx = m_spriteW / static_cast<float>(ts.x);
            float sy = m_spriteH / static_cast<float>(ts.y);
            m_sprite.setTexture(*tex, true);
            m_sprite.setScale({ sx, sy });
            m_sprite.setPosition(position);
        }
        window.draw(m_sprite);
    } else {
        // Fallback: purple rectangle
        sf::RectangleShape fb({ m_spriteW, m_spriteH });
        fb.setFillColor(sf::Color(120, 40, 160));
        fb.setOutlineColor(sf::Color::White);
        fb.setOutlineThickness(2.f);
        fb.setPosition(position);
        window.draw(fb);

        // Label
        // (font not available here; fallback shape is sufficient)
    }
}

// ---------------------------------------------------------------
void Mogera::setPosition(sf::Vector2f pos) {
    position = pos;
    syncHitBox();
    syncSprite();
}

// ---------------------------------------------------------------
void Mogera::takeSnowballHit() {
    if (m_bossState == BossState::Dying || m_bossState == BossState::Dead) return;

    m_hitsRemaining--;
    std::cout << "[Mogera] Hit! Remaining HP: " << m_hitsRemaining << "/" << m_maxHits << "\n";

    if (m_hitsRemaining <= 0) {
        m_hitsRemaining = 0;
        m_canAttack     = false;
        m_bossState     = BossState::Dying;
        m_stateTimer    = DEATH_DURATION;
        m_rewardPending = true;
        // Begin falling to ground
        velocity.y = 0.f;
        syncSprite();
        std::cout << "[Mogera] DEFEATED — starting death sequence\n";
    }
}