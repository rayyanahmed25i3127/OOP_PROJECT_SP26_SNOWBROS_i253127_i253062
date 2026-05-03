#include "projectiles/Knife.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>

Knife::Knife(sf::Vector2f pos, sf::Vector2f direction)
    : Projectile(pos)
    , m_texture()
    , m_textureLoaded(false)
    , m_sprite(m_texture)
    , m_direction(direction)
    , m_speed(KNIFE_SPEED)
    , m_lifetime(0.f)
{
    // Normalize direction
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f) { m_direction.x /= len; m_direction.y /= len; }

    if (std::FILE* f = std::fopen("assets/sprites/knife.png", "rb")) {
        std::fclose(f);
        if (m_texture.loadFromFile("assets/sprites/knife.png")) {
            m_textureLoaded = true;
            m_sprite.setTexture(m_texture, true);
            auto ts = m_texture.getSize();
            if (ts.x > 0 && ts.y > 0)
                m_sprite.setScale({20.f / (float)ts.x, 20.f / (float)ts.y});
        }
    } else {
        std::cerr << "[Knife] missing knife.png\n";
    }

    hitBox.size     = {14.f, 14.f};
    hitBox.position = pos;
    m_sprite.setPosition(pos);
}

void Knife::update(float dt) {
    m_lifetime += dt;
    if (m_lifetime >= MAX_LIFETIME) { alive = false; return; }

    position.x += m_direction.x * m_speed * dt;
    position.y += m_direction.y * m_speed * dt;

    // Off-screen check
    if (position.x < -40.f || position.x > 840.f ||
        position.y < -40.f || position.y > 640.f) {
        alive = false; return;
    }

    hitBox.position = position;
    m_sprite.setPosition(position);
}

void Knife::draw(sf::RenderWindow& window) {
    if (!alive) return;
    if (m_textureLoaded) window.draw(m_sprite);
    else {
        sf::RectangleShape r({14.f, 4.f});
        r.setPosition(position);
        r.setFillColor(sf::Color(200, 200, 60));
        window.draw(r);
    }
}

void Knife::drawHitBoxDebug(sf::RenderWindow& window, sf::Color c) {
    sf::RectangleShape r({hitBox.size.x, hitBox.size.y});
    r.setPosition({hitBox.position.x, hitBox.position.y});
    r.setFillColor(sf::Color::Transparent);
    r.setOutlineColor(c);
    r.setOutlineThickness(1.f);
    window.draw(r);
}