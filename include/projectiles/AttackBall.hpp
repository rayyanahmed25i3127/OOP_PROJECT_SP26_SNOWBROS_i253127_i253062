#pragma once
#include "projectiles/Projectile.hpp"
#include <string>

/**
 * @brief Player's thrown attack projectile (spec §7.1).
 *
 * CHANGES in Phase 3:
 *   - Real sprite (attackball_blue.png / attackball_orange.png) replaces circle
 *   - No screen wrap — flies ~192px (2 inches) then dies
 *   - Rolling wrap-around behavior belongs to the rolling snowball, not this
 *
 * Lifecycle:
 *   Spawned by PlayState when player throws. Flies horizontally. Dies on:
 *     (a) distance travelled >= m_maxDistance
 *     (b) enemy hit (PlayState marks alive=false after registering hit)
 *
 * INHERITANCE:
 *   Entity → Projectile → AttackBall
 */
class AttackBall : public Projectile {
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    sf::CircleShape m_fallbackShape;  // if sprite fails to load

    float m_distanceTravelled;
    float m_maxDistance;
    float m_speed;
    bool  m_facingRight;

    float m_spriteWidth;
    float m_spriteHeight;

public:
    AttackBall(sf::Vector2f pos, bool facingRight,
               const std::string& texturePath = "assets/sprites/attackball_blue.png");

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;

    bool isFacingRight() const { return m_facingRight; }
};