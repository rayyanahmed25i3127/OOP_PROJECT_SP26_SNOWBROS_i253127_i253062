#include "powerups/Diamond.hpp"
#include <iostream>
#include <cstdio>

Diamond::Diamond(sf::Vector2f pos)
    : Entity(pos) 
    , m_texture()        
    , m_sprite(m_texture) 
    , m_textureLoaded(false)
    , m_fallback(10.0f)
    , m_lifetime(DIAMOND_TIMEOUT)
    , m_gravity(800.0f)
    , m_spriteWidth(20.0f)
    , m_spriteHeight(20.0f)
    , m_hitOffsetX(2.0f)
    , m_hitOffsetY(2.0f)
{
    const char* texPath = "assets/sprites/diamond.png";

    std::FILE* check = std::fopen(texPath, "r");
    if (check) {
        std::fclose(check);
        if (m_texture.loadFromFile(texPath)) {
            m_textureLoaded = true;
            
            // SFML 3 Visibility Fix: Refresh sprite with loaded texture
            m_sprite.setTexture(m_texture, true);
            
            sf::Vector2u texSize = m_texture.getSize();
            float scaleX = m_spriteWidth  / static_cast<float>(texSize.x);
            float scaleY = m_spriteHeight / static_cast<float>(texSize.y);
            m_sprite.setScale({scaleX, scaleY});
            
            std::cout << "[Diamond] Loaded texture: " << texPath << "\n";
        } else {
            std::cout << "[Diamond] Warning: Failed to load " << texPath << "\n";
        }
    } else {
        std::cout << "[Diamond] Warning: Cannot open " << texPath << "\n";
    }

    m_fallback.setFillColor(sf::Color(255, 215, 0));  // Gold
    m_fallback.setOutlineColor(sf::Color::White);
    m_fallback.setOutlineThickness(1.0f);

    syncSprite();

    // SFML 3 Hitbox Structure
    hitBox.size.x = m_spriteWidth  - 2.0f * m_hitOffsetX;
    hitBox.size.y = m_spriteHeight - 2.0f * m_hitOffsetY;
    hitBox.position.x = position.x + m_hitOffsetX;
    hitBox.position.y = position.y + m_hitOffsetY;

    std::cout << "[Diamond] Spawned at (" << position.x << ", " << position.y << ")\n";
}

void Diamond::update(float dt) {
    m_lifetime -= dt;
    if (m_lifetime <= 0.0f) {
        alive = false;
        return;
    }

    if (m_lifetime <= 2.0f) {
        int cyclePhase = static_cast<int>((DIAMOND_TIMEOUT - m_lifetime) / 0.1f);
        bool visible = (cyclePhase % 2 == 0);
        if (m_textureLoaded) {
            sf::Color c = m_sprite.getColor();
            c.a = static_cast<uint8_t>(visible ? 255 : 100);
            m_sprite.setColor(c);
        }
        m_fallback.setFillColor(visible ? sf::Color(255, 215, 0) : sf::Color(255, 215, 0, 100));
    }

    velocity.y += m_gravity * dt;
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    syncSprite();

    hitBox.position.x = position.x + m_hitOffsetX;
    hitBox.position.y = position.y + m_hitOffsetY;
}

void Diamond::draw(sf::RenderWindow& window) {
    if (m_textureLoaded) {
        window.draw(m_sprite);
    } else {
        window.draw(m_fallback);
    }
}

void Diamond::setPosition(sf::Vector2f pos) {
    position = pos;
    syncSprite();
    hitBox.position.x = position.x + m_hitOffsetX;
    hitBox.position.y = position.y + m_hitOffsetY;
}

void Diamond::syncSprite() {
    if (m_textureLoaded) {
        m_sprite.setPosition(position);
    }
    m_fallback.setPosition(position);
}