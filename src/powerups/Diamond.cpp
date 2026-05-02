#include "powerups/Diamond.hpp"
#include <iostream>
#include <cstdio>

Diamond::Diamond(sf::Vector2f pos)
    : Entity(pos)
    , m_sprite(m_texture)
    , m_textureLoaded(false)
    , m_fallback(10.0f)
    , m_lifetime(DIAMOND_TIMEOUT)
    , m_isBonusDiamond(false)
    , m_gravity(800.0f)
    , m_spriteWidth(20.0f)
    , m_spriteHeight(20.0f)
    , m_hitOffsetX(2.0f)
    , m_hitOffsetY(2.0f)
{
    if (std::FILE* f = std::fopen("assets/sprites/diamond.png", "rb")) {
        std::fclose(f);
        if (m_texture.loadFromFile("assets/sprites/diamond.png")) {
            m_textureLoaded = true;
            m_sprite.setTexture(m_texture, true);
        }
    }

    m_fallback.setFillColor(sf::Color(255, 215, 0));
    m_fallback.setOutlineColor(sf::Color::White);
    m_fallback.setOutlineThickness(1.f);

    hitBox.size     = { m_spriteWidth - 4.f, m_spriteHeight - 4.f };
    hitBox.position = { pos.x + m_hitOffsetX, pos.y + m_hitOffsetY };

    syncSprite();
}

void Diamond::syncSprite() {
    if (m_textureLoaded) {
        auto ts = m_texture.getSize();
        if (ts.x > 0 && ts.y > 0) {
            m_sprite.setScale({ m_spriteWidth  / static_cast<float>(ts.x),
                                m_spriteHeight / static_cast<float>(ts.y) });
            m_sprite.setPosition(position);
        }
    } else {
        m_fallback.setPosition({ position.x + m_hitOffsetX,
                                 position.y + m_hitOffsetY });
    }
}

void Diamond::update(float dt) {
    if (!m_isBonusDiamond) {
        m_lifetime -= dt;
        if (m_lifetime <= 0.f) { alive = false; return; }

        // Blink last 2 seconds
        if (m_lifetime <= 2.f) {
            bool show = (static_cast<int>(m_lifetime * 8.f) % 2 == 0);
            sf::Color c = m_textureLoaded ? m_sprite.getColor()
                                          : m_fallback.getFillColor();
            c.a = show ? 255 : 80;
            if (m_textureLoaded) m_sprite.setColor(c);
            else m_fallback.setFillColor(c);
        }
    }

    velocity.y += m_gravity * dt;
    position   += velocity * dt;
    hitBox.position = { position.x + m_hitOffsetX, position.y + m_hitOffsetY };
    syncSprite();
}

void Diamond::draw(sf::RenderWindow& window) {
    if (m_textureLoaded) window.draw(m_sprite);
    else                 window.draw(m_fallback);
}

void Diamond::setPosition(sf::Vector2f pos) {
    position = pos;
    hitBox.position = { pos.x + m_hitOffsetX, pos.y + m_hitOffsetY };
    syncSprite();
}