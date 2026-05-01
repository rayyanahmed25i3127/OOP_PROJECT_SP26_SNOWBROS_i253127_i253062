#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class Player : public Entity {
public:
    static const int MAX_LIVES = 3;   // hard ceiling used by addLife() and shop

private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;

    int   m_lives;
    float m_invincibleTimer;
    bool  m_blinkVisible;

    float m_throwCooldown;
    float m_throwInterval;
    bool  m_wantsToThrow;

    float speed;
    float jumpForce;
    float gravity;
    bool  onGround;
    bool  m_facingRight;

    bool  m_balloonMode;
    float m_balloonGravity;

public:
    explicit Player(sf::Vector2f pos, int characterIndex = 0);

    void setPosition(sf::Vector2f pos) override;

    void handleInput();
    void applyGravity(float dt);

    bool wantsToThrow()        const { return m_wantsToThrow; }
    void consumeThrowRequest()       { m_wantsToThrow = false; m_throwCooldown = m_throwInterval; }
    bool isFacingRight()       const { return m_facingRight; }

    virtual void update(float dt)              override;
    virtual void draw(sf::RenderWindow& window) override;

    void setOnGround(bool value) override { onGround = value; }
    bool isOnGround()      const { return onGround; }
    int  getLives()        const { return m_lives; }
    bool isInvincible()    const { return m_invincibleTimer > 0.f; }
    void loseLife();
    void respawn(sf::Vector2f spawnPos);

    // Adds one life, capped at MAX_LIVES (3).
    // Returns true if the life was actually added, false if already at cap.
    bool addLife();

    void setBalloonMode(bool val)  { m_balloonMode = val; }
    bool isBalloonMode()     const { return m_balloonMode; }
    void setSpeedMultiplier(float multiplier);
};