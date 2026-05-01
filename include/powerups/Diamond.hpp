#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Collectible diamond dropped on every second chain kill.
 * 
 * Lifecycle:
 *   - Spawned by PlayState at chain-kill position on every 2nd chain kill.
 *   - Falls under gravity, lands on platforms.
 *   - Despawns on player-hitbox overlap (PlayState adds 15 gems).
 *   - Auto-despawns after DIAMOND_TIMEOUT seconds.
 * 
 * INHERITANCE:
 *   Entity → Diamond.
 */
class Diamond : public Entity {
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    sf::CircleShape m_fallback;   // shown if asset missing

    float m_lifetime;             // seconds remaining before auto-despawn
    float m_gravity;
    float m_spriteWidth;
    float m_spriteHeight;
    float m_hitOffsetX;
    float m_hitOffsetY;

    void syncSprite();

public:
    static constexpr float DIAMOND_TIMEOUT = 8.0f;  // 8 seconds lifetime
    static constexpr int   GEM_VALUE = 15;          // gives 15 gems when picked

    Diamond(sf::Vector2f pos);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;
    void setOnGround(bool /*v*/) override {}

    bool isExpired() const { return m_lifetime <= 0.0f; }
};