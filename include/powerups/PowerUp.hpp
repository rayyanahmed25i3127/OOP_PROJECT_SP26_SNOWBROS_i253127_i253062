#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Pickup item dropped by chain-killed enemies (spec Â§8).
 *
 * Lifecycle:
 *   - Spawned by PlayState at a chain-kill position with a randomly
 *     chosen Type.
 *   - Falls under gravity, lands on platforms (routed through
 *     CollisionDetector â€” same code path as Player/Enemy).
 *   - Despawns on player-hitbox overlap (PlayState applies the effect).
 *   - Auto-despawns after PICKUP_TIMEOUT seconds (so old drops don't
 *     clutter the level forever).
 *
 * INHERITANCE (spec Â§11.1 â€” abstract bases for Enemy / PowerUp / Projectile):
 *   Entity â†’ PowerUp.
 *
 * Note: spec Â§8.2 lists 4 Level-1 power-ups (Speed Boost, Snowball Power,
 * Distance Increase, Balloon Mode). Extra Life is shop-only per Â§8.4.
 */
class PowerUp : public Entity {
public:
    enum class Type {
        SpeedBoost,        // 15s â€” +50% movement speed
        SnowballPower,     // until level end â€” 1-hit encase
        DistanceIncrease,  // until level end â€” snowball travels full screen
        BalloonMode,       // 10s â€” float upward, ground enemies can't harm
        Count_             // sentinel for random pick
    };

private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    sf::CircleShape m_fallback;   // shown if asset missing

    Type  m_type;
    float m_lifetime;             // seconds remaining before auto-despawn
    float m_gravity;
    float m_spriteWidth;
    float m_spriteHeight;
    float m_hitOffsetX;
    float m_hitOffsetY;

    void syncSprite();

public:
    PowerUp(sf::Vector2f pos, Type type);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;
    void setOnGround(bool /*v*/) override {}   // not needed; we read landed via vel.y zeroing in CollisionDetector

    Type getType() const { return m_type; }

    static const char* typeName(Type t);
    static const char* texturePath(Type t);
};