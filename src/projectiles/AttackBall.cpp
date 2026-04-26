#include "projectiles/AttackBall.hpp"
#include <cstdio>
#include <iostream>

namespace {
    // Attack ball design parameters (spec §7.1 — "travels 2-2.5 inches").
    // At typical 96 DPI, 2 inches ≈ 192 pixels. Use 200 as round number.
    const float ATTACKBALL_SPEED    = 700.f;   // snappier arcade feel
    const float ATTACKBALL_MAX_DIST = 220.f;
    const float SCREEN_WIDTH        = 800.f;   // for Distance Increase power-up

    // Visual / physics size of the projectile sprite
    const float ATTACKBALL_W = 20.f;
    const float ATTACKBALL_H = 20.f;

    // Hit-box is a bit tighter than the sprite
    const float ATTACKBALL_HIT_W = 16.f;
    const float ATTACKBALL_HIT_H = 16.f;
}

AttackBall::AttackBall(sf::Vector2f pos, bool facingRight,
                       const std::string& texturePath)
    : Projectile(pos)
    , m_sprite(m_texture)
    , m_textureLoaded(false)
    , m_fallbackShape(ATTACKBALL_HIT_W * 0.5f)
    , m_distanceTravelled(0.f)
    , m_maxDistance(ATTACKBALL_MAX_DIST)
    , m_speed(ATTACKBALL_SPEED)
    , m_facingRight(facingRight)
    , m_maxRangeMode(false)  // ===== ADD THIS LINE =====
    , m_spriteWidth(ATTACKBALL_W)
    , m_spriteHeight(ATTACKBALL_H)
{
    velocity.x = facingRight ? m_speed : -m_speed;
    velocity.y = 0.f;

    // Load sprite with fopen guard (SFML 3 Windows quirk)
    if (std::FILE* f = std::fopen(texturePath.c_str(), "rb")) {
        std::fclose(f);
        if (m_texture.loadFromFile(texturePath)) {
            m_textureLoaded = true;
            m_sprite.setTexture(m_texture, true);
        } else {
            std::cerr << "[AttackBall] failed to decode " << texturePath << "\n";
        }
    } else {
        std::cerr << "[AttackBall] missing " << texturePath
                  << " — falling back to circle\n";
    }

    // Configure fallback circle (used only if sprite didn't load)
    m_fallbackShape.setFillColor(sf::Color(245, 250, 255));
    m_fallbackShape.setOutlineColor(sf::Color(120, 140, 180));
    m_fallbackShape.setOutlineThickness(1.f);

    hitBox.size     = { ATTACKBALL_HIT_W, ATTACKBALL_HIT_H };
    hitBox.position = pos;

    setPosition(pos);
}

void AttackBall::update(float dt) {
    float dx = velocity.x * dt;
    position.x += dx;
    m_distanceTravelled += (dx < 0 ? -dx : dx);

    setPosition(position);

    // ===== POWER-UP: Distance Increase =====
    // When max-range mode is active, snowball travels full screen width (800px)
    // Otherwise, uses default range (220px)
    float effectiveRange = m_maxRangeMode ? SCREEN_WIDTH : m_maxDistance;
    
    if (m_distanceTravelled >= effectiveRange) {
        alive = false;
    }
}

void AttackBall::draw(sf::RenderWindow& window) {
    if (m_textureLoaded) {
        window.draw(m_sprite);
    } else {
        window.draw(m_fallbackShape);
    }
}

void AttackBall::setPosition(sf::Vector2f pos) {
    position = pos;

    if (m_textureLoaded) {
        auto texSize = m_texture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float sx = m_spriteWidth  / static_cast<float>(texSize.x);
            float sy = m_spriteHeight / static_cast<float>(texSize.y);

            m_sprite.setOrigin({ static_cast<float>(texSize.x) / 2.f, 0.f });

            if (m_facingRight) m_sprite.setScale({  sx, sy });
            else               m_sprite.setScale({ -sx, sy });

            m_sprite.setPosition({ pos.x + m_spriteWidth / 2.f, pos.y });
        }
    } else {
        m_fallbackShape.setPosition(pos);
    }

    hitBox.position = pos;
}