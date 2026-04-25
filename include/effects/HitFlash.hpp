#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Brief visual impact effect when an attack ball hits an enemy.
 *
 * Spec (from our design chat): attack ball dies on hit AND spawns a small
 * crescent flash at the impact point that fades out over ~100ms. This is
 * a transient visual — no physics, no collision.
 *
 * Uses the same attack-ball sprite rendered at 60% size so we don't need
 * a separate asset. Rotates slightly over its lifetime for a "pop" feel.
 */
class HitFlash {
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    float m_lifetime;     // remaining ms
    float m_maxLifetime;  // reset value for alpha interpolation
    bool  m_alive;

    sf::Vector2f m_position;

public:
    explicit HitFlash(sf::Vector2f pos,
                      const std::string& texturePath = "assets/sprites/hit_flash_blue.png");

    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isAlive() const { return m_alive; }
};