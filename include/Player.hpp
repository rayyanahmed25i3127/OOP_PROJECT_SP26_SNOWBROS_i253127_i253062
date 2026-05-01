#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief The playable character (Nick).
 *
 * Uses direct texture-swap animation for individual PNG frames.
 * Each animation state has its own texture array. On each frame,
 * the correct texture is applied to the sprite directly.
 */
class Player : public Entity {
public:
    static const int MAX_LIVES = 3;   // hard ceiling used by addLife() and shop

private:
    // ----- Textures (persistent — must outlive the sprite) -----
    sf::Texture m_idleTexture;
    sf::Texture m_walkTextures[3];      // 3 walk frames
    sf::Texture m_jumpTexture;          // 1 jump/fall frame
    sf::Texture m_throwTextures[2];     // 2 throw frames

    bool m_idleLoaded;
    bool m_walkLoaded;
    bool m_jumpLoaded;
    bool m_throwLoaded;

    sf::Sprite  m_sprite;

// ----- Animation timer -----
int   m_walkFrame;      // current walk frame index (0-2)
float m_walkTimer;      // counts up, switches frame
int   m_throwFrame;     // current throw frame index (0-1)
float m_throwTimer;     // counts up
bool  m_isThrowing;     // throw animation flag

// ----- Player State -----
int   m_lives;
float m_invincibleTimer;
bool  m_blinkVisible;

float m_throwCooldown;
float m_throwInterval;  // 0.18s
    bool  m_wantsToThrow;

    float speed;
    float jumpForce;
    float gravity;
    bool  onGround;
    bool  m_facingRight;

// ===== POWER-UP: Balloon Mode =====
bool  m_balloonMode;
float m_balloonGravity;

// ----- Helpers -----
void loadAnimations();
void updateAnimation(float dt);
void applySpriteTransform();   // scale + flip + position

public:
    explicit Player(sf::Vector2f pos, int characterIndex = 0);

    void setPosition(sf::Vector2f pos) override;

    void handleInput();
    void applyGravity(float dt);

bool wantsToThrow() const { return m_wantsToThrow; }

void consumeThrowRequest() {
    m_wantsToThrow = false;
    m_throwCooldown = m_throwInterval;

    // 🔥 Keep animation trigger
    m_isThrowing = true;
    m_throwFrame = 0;
    m_throwTimer = 0.f;
}

bool isFacingRight() const { return m_facingRight; }

virtual void update(float dt) override;
virtual void draw(sf::RenderWindow& window) override;

void setOnGround(bool value) override { onGround = value; }
bool isOnGround() const { return onGround; }

int getLives() const { return m_lives; }
bool isInvincible() const { return m_invincibleTimer > 0.f; }

void loseLife();
void respawn(sf::Vector2f spawnPos);

// 🔥 KEEP THIS (your feature)
bool addLife();

void setBalloonMode(bool val) { m_balloonMode = val; }
bool isBalloonMode() const { return m_balloonMode; }
    void setSpeedMultiplier(float multiplier);
};