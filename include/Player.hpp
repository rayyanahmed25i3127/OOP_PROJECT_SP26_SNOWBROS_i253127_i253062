#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief The playable character.
 *
 * characterIndex selects which sprite to load at construction time:
 *   0 = Blue  (player_blue_idle.png)
 *   1 = Red   (player_red_idle.png)
 *   2 = Modi  (player_modi_idle.png)
 *
 * PlayState passes m_characterIndex down from CharacterSelectState.
 */
class Player : public Entity {
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
    // ── characterIndex selects the sprite (0=Blue, 1=Red, 2=Modi) ──────
    explicit Player(sf::Vector2f pos, int characterIndex = 0);

    void setPosition(sf::Vector2f pos) override;

    void handleInput();
    void applyGravity(float dt);

    bool wantsToThrow()     const { return m_wantsToThrow; }
    void consumeThrowRequest()    { m_wantsToThrow = false; m_throwCooldown = m_throwInterval; }
    bool isFacingRight()    const { return m_facingRight; }

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;

    void setOnGround(bool value) override { onGround = value; }
    bool isOnGround()       const { return onGround; }
    int  getLives()         const { return m_lives; }
    bool isInvincible()     const { return m_invincibleTimer > 0.f; }
    void loseLife();
    void respawn(sf::Vector2f spawnPos);

    void setBalloonMode(bool val)  { m_balloonMode = val; }
    bool isBalloonMode()    const  { return m_balloonMode; }

    void setSpeedMultiplier(float multiplier);
};