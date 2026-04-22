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

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;

    // Called by CollisionDetector after resolving the player's movement
    // against the world. Determines whether jumping is available next frame.
    void setOnGround(bool value) { onGround = value; }
    bool isOnGround() const      { return onGround; }
};