#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class Player : public Entity {
private:
    struct Animation {
        sf::Texture  frameTextures[8];
        sf::Texture* activeTexture;
        float frameDurations[8];

        int   frameCount    = 0;
        int   frameW        = 0;
        int   frameH        = 0;
        float frameDuration = 0.15f;
        bool  loaded        = false;

        sf::Texture& texture = frameTextures[0];

        Animation() : activeTexture(nullptr) {
            for (int i = 0; i < 8; ++i) frameDurations[i] = 0.15f;
        }

        Animation(const Animation&)            = delete;
        Animation& operator=(const Animation&) = delete;
    };

    enum class AnimState { Idle = 0, Walk = 1, Jump = 2, Throw = 3 };

    Animation  m_animations[4];
    AnimState  m_currentAnim;
    int        m_currentFrame;
    float      m_animTimer;

    sf::Sprite m_sprite;

    int   m_lives;
    float m_invincibleTimer;
    bool  m_blinkVisible;
    float m_throwCooldown;
    float m_throwInterval;
    bool  m_wantsToThrow;
    bool  m_autoAttack;
    float m_autoAttackTimer;

    float speed;
    float jumpForce;
    float gravity;
    bool  onGround;
    bool  m_facingRight;
    bool  m_balloonMode;
    float m_balloonGravity;

    void loadAnimations(int characterIndex);
    void updateAnimation(float dt);
    void applySpriteTransform();

public:
    static const int MAX_LIVES = 99;

    explicit Player(sf::Vector2f pos, int characterIndex = 0);
    void setPosition(sf::Vector2f pos) override;
    void handleInput();
    void applyGravity(float dt);

    bool wantsToThrow() const     { return m_wantsToThrow; }
    bool isAutoAttack() const     { return m_autoAttack; }
    void consumeThrowRequest()    { m_wantsToThrow = false; m_throwCooldown = m_throwInterval; }
    bool isFacingRight() const    { return m_facingRight; }

    virtual void update(float dt)              override;
    virtual void draw(sf::RenderWindow& window) override;

    void setOnGround(bool value) override { onGround = value; }
    bool isOnGround()  const { return onGround; }
    int  getLives()    const { return m_lives; }
    bool isInvincible() const { return m_invincibleTimer > 0.f; }
    void loseLife();
    void addLife() { ++m_lives; }
    void respawn(sf::Vector2f spawnPos);
    void setBalloonMode(bool val) { m_balloonMode = val; }
    bool isBalloonMode() const    { return m_balloonMode; }
    void setSpeedMultiplier(float multiplier);
};