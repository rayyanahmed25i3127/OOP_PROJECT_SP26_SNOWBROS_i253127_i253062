#include "powerups/PowerUp.hpp"
#include <cstdio>
#include <iostream>

namespace {
    const float SPRITE_W = 22.f;
    const float SPRITE_H = 22.f;
    const float HIT_W    = 20.f;
    const float HIT_H    = 20.f;
    const float GRAVITY  = 500.f;       // gentler fall than enemies — feels floatier
    const float LIFETIME = 8.0f;        // seconds before auto-despawn
}

const char* PowerUp::typeName(Type t) {
    switch (t) {
        case Type::SpeedBoost:       return "SPEED";
        case Type::SnowballPower:    return "SNOWBALL POWER";
        case Type::DistanceIncrease: return "DISTANCE";
        case Type::BalloonMode:      return "BALLOON";
        default:                     return "?";
    }
}

const char* PowerUp::texturePath(Type t) {
    switch (t) {
        case Type::SpeedBoost:       return "assets/sprites/powerup_speed.png";     // Blue potion
        case Type::SnowballPower:    return "assets/sprites/powerup_snowball.png";  // Snowman head
        case Type::DistanceIncrease: return "assets/sprites/powerup_distance.png";  // Attack ball icon
        case Type::BalloonMode:      return "assets/sprites/powerup_balloon.png";   // Balloon/duck
        default:                     return "";
    }
}

PowerUp::PowerUp(sf::Vector2f pos, Type type)
    : Entity(pos)
    , m_sprite(m_texture)
    , m_textureLoaded(false)
    , m_fallback(HIT_W * 0.5f)
    , m_type(type)
    , m_lifetime(LIFETIME)
    , m_gravity(GRAVITY)
    , m_spriteWidth(SPRITE_W)
    , m_spriteHeight(SPRITE_H)
    , m_hitOffsetX((SPRITE_W - HIT_W) / 2.f)
    , m_hitOffsetY((SPRITE_H - HIT_H) / 2.f)
{
    velocity = { 0.f, -120.f };   // small upward pop on spawn (juicy feel)

    const std::string path = texturePath(type);
    if (std::FILE* f = std::fopen(path.c_str(), "rb")) {
        std::fclose(f);
        if (m_texture.loadFromFile(path)) {
            m_textureLoaded = true;
            m_sprite.setTexture(m_texture, true);
        } else {
            std::cerr << "[PowerUp] decode fail: " << path << "\n";
        }
    } else {
        std::cerr << "[PowerUp] missing: " << path << "\n";
    }

    // Fallback color per type so debug-fallback is still informative
    sf::Color tint;
    switch (type) {
        case Type::SpeedBoost:       tint = sf::Color(120, 200, 255); break;
        case Type::SnowballPower:    tint = sf::Color(255, 120, 200); break;
        case Type::DistanceIncrease: tint = sf::Color(80,  140, 255); break;
        case Type::BalloonMode:      tint = sf::Color(180, 220, 255); break;
        default:                     tint = sf::Color::White;         break;
    }
    m_fallback.setFillColor(tint);
    m_fallback.setOutlineColor(sf::Color::Black);
    m_fallback.setOutlineThickness(1.f);

    hitBox.size     = { HIT_W, HIT_H };
    hitBox.position = { pos.x + m_hitOffsetX, pos.y + m_hitOffsetY };

    setPosition(pos);
}

void PowerUp::syncSprite() {
    if (m_textureLoaded) {
        auto ts = m_texture.getSize();
        if (ts.x > 0 && ts.y > 0) {
            float sx = m_spriteWidth  / static_cast<float>(ts.x);
            float sy = m_spriteHeight / static_cast<float>(ts.y);
            m_sprite.setScale({ sx, sy });
            m_sprite.setPosition(position);
        }
    } else {
        m_fallback.setPosition({ position.x + m_hitOffsetX,
                                 position.y + m_hitOffsetY });
    }
}

void PowerUp::update(float dt) {
    velocity.y += m_gravity * dt;
    position += velocity * dt;
    hitBox.position = { position.x + m_hitOffsetX,
                        position.y + m_hitOffsetY };
    syncSprite();

    m_lifetime -= dt;
    if (m_lifetime <= 0.f) alive = false;
}

void PowerUp::draw(sf::RenderWindow& window) {
    // Blink the icon during the last 2 seconds before despawn (warning).
    bool blinkHide = (m_lifetime < 2.0f) &&
                     (static_cast<int>(m_lifetime * 8.f) % 2 == 0);
    if (blinkHide) return;

    if (m_textureLoaded) window.draw(m_sprite);
    else                 window.draw(m_fallback);
}

void PowerUp::setPosition(sf::Vector2f pos) {
    position = pos;
    hitBox.position = { pos.x + m_hitOffsetX, pos.y + m_hitOffsetY };
    syncSprite();
}