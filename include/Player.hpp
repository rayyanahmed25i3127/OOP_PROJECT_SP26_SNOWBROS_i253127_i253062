#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief The playable character (Nick).
 *
 * Inherits from Entity (position, velocity, hitbox, alive).
 * Uses composition with a single static sprite for now.
 *
 * Physics update flow:
 *   1. handleInput() reads keyboard, sets horizontal velocity + jump
 *   2. applyGravity() accelerates downward
 *   3. update() applies velocity to position
 *   4. CollisionDetector resolves against walls and platforms (external)
 *   5. setOnGround() informs the player whether they can jump next frame
 *
 * Note: ground/wall/platform collision is NOT handled inside Player.
 * Per spec 7.2 it's routed through a dedicated CollisionDetector.
 */
class Player : public Entity {
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;

    int   m_lives;
    float m_invincibleTimer;   // seconds remaining; 0 = not invincible
    bool  m_blinkVisible;      // flips during invincibility for blink effect

    float m_throwCooldown;     // seconds until next throw allowed
    float m_throwInterval;     // cooldown reset value (0.4s)
    bool  m_wantsToThrow;      // set true on throw-key press, cleared by PlayState after spawning snowball

    float speed;
    float jumpForce;
    float gravity;
    bool  onGround;
    bool  m_facingRight;

public:
    Player(sf::Vector2f pos);
    void setPosition(sf::Vector2f pos) override;
    // Override so hitbox stays offset correctly after external repositioning
    // void setPosition(sf::Vector2f pos);

    void handleInput();
    void applyGravity(float dt);

    bool wantsToThrow() const     { return m_wantsToThrow; }
    void consumeThrowRequest()    { m_wantsToThrow = false; m_throwCooldown = m_throwInterval; }
    bool isFacingRight() const    { return m_facingRight; }
    
    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;

    // Called by CollisionDetector after resolving the player's movement
    // against the world. Determines whether jumping is available next frame.
    void setOnGround(bool value) override { onGround = value; }
    bool isOnGround() const      { return onGround; }
    int  getLives() const        { return m_lives; }
    bool isInvincible() const    { return m_invincibleTimer > 0.f; }
    void loseLife();             // called on enemy contact; triggers invincibility + respawn
    void respawn(sf::Vector2f spawnPos);
};