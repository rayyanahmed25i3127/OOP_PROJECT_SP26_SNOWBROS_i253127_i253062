#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class Player : public Entity {
private:
    // ---------------------------------------------------------------
    //  Per-state animation data.
    //
    //  Rayyan's assets are individual PNGs per frame (not sprite sheets).
    //  Each state therefore stores up to 8 separate sf::Texture objects
    //  plus a per-frame duration array.
    //
    //  activeTexture is a non-owning pointer to whichever frameTextures[]
    //  entry is currently displayed â€” avoids any sf::Texture copy.
    //
    //  The single `texture` member from the original version is kept
    //  (as frameTextures[0]) so the m_sprite constructor reference in
    //  the initialiser list remains valid (SFML 3 deleted sf::Sprite's
    //  default constructor and requires a texture at construction time).
    // ---------------------------------------------------------------
    struct Animation {
        sf::Texture  frameTextures[8]; // one texture per individual-PNG frame
        sf::Texture* activeTexture;    // non-owning; points into frameTextures[]
                                       // (or into idle's array for fallback states)
        float frameDurations[8];       // per-frame display time in seconds

        int   frameCount    = 0;
        int   frameW        = 0;       // pixel width of one frame (full PNG width)
        int   frameH        = 0;       // pixel height of one frame
        float frameDuration = 0.15f;   // fallback uniform duration
        bool  loaded        = false;

        // Convenience: expose first texture as `texture` so that the
        // m_sprite(m_animations[0].texture) initialiser in the ctor still
        // compiles.  frameTextures[0] IS the same object â€” this is just an
        // alias via reference.
        sf::Texture& texture = frameTextures[0];

        Animation() : activeTexture(nullptr) {
            for (int i = 0; i < 8; ++i) frameDurations[i] = 0.15f;
        }

        // Non-copyable (sf::Texture is non-copyable)
        Animation(const Animation&)            = delete;
        Animation& operator=(const Animation&) = delete;
    };

    enum class AnimState { Idle = 0, Walk = 1, Jump = 2, Throw = 3 };

    // m_animations MUST be declared BEFORE m_sprite â€” C++ guarantees
    // members are initialised in declaration order, so m_animations[0]
    // is fully constructed before m_sprite's constructor runs.
    Animation  m_animations[4];   // Idle, Walk, Jump, Throw
    AnimState  m_currentAnim;
    int        m_currentFrame;
    float      m_animTimer;

    sf::Sprite m_sprite;          // initialised with m_animations[0].texture
                                   // (= m_animations[0].frameTextures[0])

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

    void loadAnimations(int characterIndex);
    void updateAnimation(float dt);
    void applySpriteTransform();

public:
    static const int MAX_LIVES = 99;  // effectively unlimited

    explicit Player(sf::Vector2f pos, int characterIndex = 0);
    void setPosition(sf::Vector2f pos) override;
    void handleInput();
    void applyGravity(float dt);

    bool wantsToThrow()       const { return m_wantsToThrow; }
    void consumeThrowRequest()      { m_wantsToThrow = false; m_throwCooldown = m_throwInterval; }
    bool isFacingRight()      const { return m_facingRight; }

    virtual void update(float dt)              override;
    virtual void draw(sf::RenderWindow& window) override;

    void setOnGround(bool value) override { onGround = value; }
    bool isOnGround()  const { return onGround; }
    int  getLives()    const { return m_lives; }
    bool isInvincible() const { return m_invincibleTimer > 0.f; }
    void loseLife();
    void addLife() { ++m_lives; }  // no cap: extra life can be bought repeatedly
    void respawn(sf::Vector2f spawnPos);
    void setBalloonMode(bool val) { m_balloonMode = val; }
    bool isBalloonMode() const    { return m_balloonMode; }
    void setSpeedMultiplier(float multiplier);
};