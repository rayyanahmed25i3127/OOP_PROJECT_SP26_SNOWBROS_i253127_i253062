#include "effects/HitFlash.hpp"
#include <cstdio>
#include <cstdint>
#include <iostream>

namespace {
    const float FLASH_DURATION = 0.18f;   // 100ms
    // Narrow-spark sprite (hit_flash_blue.png) is 16x20 source —
    // render slightly larger than before so the thin shape still reads.
    const float FLASH_WIDTH    = 14.f;
    const float FLASH_HEIGHT   = 18.f;
}

HitFlash::HitFlash(sf::Vector2f pos, const std::string& texturePath)
    : m_sprite(m_texture)
    , m_textureLoaded(false)
    , m_lifetime(FLASH_DURATION)
    , m_maxLifetime(FLASH_DURATION)
    , m_alive(true)
    , m_position(pos)
{
    if (std::FILE* f = std::fopen(texturePath.c_str(), "rb")) {
        std::fclose(f);
        if (m_texture.loadFromFile(texturePath)) {
            m_textureLoaded = true;
            m_sprite.setTexture(m_texture, true);
            auto tsz = m_texture.getSize();
            if (tsz.x > 0 && tsz.y > 0) {
                float sx = FLASH_WIDTH  / static_cast<float>(tsz.x);
                float sy = FLASH_HEIGHT / static_cast<float>(tsz.y);
                m_sprite.setScale({ sx, sy });
            }
            m_sprite.setPosition(pos);
        }
    }
}

void HitFlash::update(float dt) {
    m_lifetime -= dt;
    if (m_lifetime <= 0.f) {
        m_alive = false;
        return;
    }
    // Fade out via sprite color alpha
    if (m_textureLoaded) {
        float a = m_lifetime / m_maxLifetime;  // 1.0 → 0.0
        std::uint8_t alpha = static_cast<std::uint8_t>(a * 255.f);
        // Brighten to near-white for "pop" look
        m_sprite.setColor(sf::Color(255, 255, 255, alpha));
    }
}

void HitFlash::draw(sf::RenderWindow& window) {
    if (m_textureLoaded) window.draw(m_sprite);
    // No fallback visual — if asset missing, flash is silently invisible
    // (game still plays fine, hit registers normally via attack-ball death).
}