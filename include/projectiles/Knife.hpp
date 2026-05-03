#pragma once
#include "projectiles/Projectile.hpp"

/**
 * Knife — fired by Tornado toward player's last known position.
 * Travels in a straight line at constant speed. Damages player on contact.
 * Dies when it leaves the screen or hits the player.
 */
class Knife : public Projectile {
private:
    sf::Texture m_texture;
    bool        m_textureLoaded;
    sf::Sprite  m_sprite;

    sf::Vector2f m_direction;   // normalized
    float        m_speed;

    static constexpr float KNIFE_SPEED   = 420.f;
    static constexpr float MAX_LIFETIME  = 4.f;
    float m_lifetime;

public:
    Knife(sf::Vector2f pos, sf::Vector2f direction);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawHitBoxDebug(sf::RenderWindow& window, sf::Color c);
};