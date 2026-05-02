#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Abstract base for every enemy in the game.
 *
 * Animation uses direct texture-swap: each frame has its own
 * persistent sf::Texture stored as a class member. No sprite sheets.
 */
class Enemy : public Entity {
public:
    enum class State {
        Alive,
        PartialEncase,     
        Snowballed,        
        Escaping75,
        Escaping50,
        Escaping25,
        Rolling,           
        Dead
    };

protected:
    // ----- BODY textures (multi-pose) -----
    sf::Texture m_idleTexture;
    sf::Texture m_trappedTexture;
    sf::Texture m_unleash1Texture;   
    sf::Texture m_unleash2Texture;   
    sf::Texture m_unleash3Texture;   

    bool m_idleLoaded;
    bool m_trappedLoaded;
    bool m_unleash1Loaded;
    bool m_unleash2Loaded;
    bool m_unleash3Loaded;

    sf::Sprite m_bodySprite;

    // ----- SNOW overlay textures -----
    sf::Texture m_snowEncase50Texture;
    sf::Texture m_snowEncase100Texture;
    sf::Texture m_snowEscape75Texture;
    sf::Texture m_snowEscape50Texture;
    sf::Texture m_snowEscape25Texture;

    bool m_snowEncase50Loaded;
    bool m_snowEncase100Loaded;
    bool m_snowEscape75Loaded;
    bool m_snowEscape50Loaded;
    bool m_snowEscape25Loaded;

    sf::Sprite m_overlaySprite;
    bool       m_overlayVisible;

    // ----- Animation textures (persistent â€” outlive the sprite) -----
    sf::Texture m_walkTextures[3];   // 3 walk frames
    sf::Texture m_jumpTexture;       // 1 jump frame (going up)
    sf::Texture m_fallTexture;       // 1 fall frame (coming down)

    bool m_walkLoaded;
    bool m_jumpLoaded;
    bool m_fallLoaded;

    // Walk animation timer
    int   m_walkFrame;               // 0-2
    float m_walkTimer;

    // ----- Layout / physics -----
    float m_spriteWidth;
    float m_spriteHeight;
    float m_overlayWidth;
    float m_overlayHeight;
    float m_hitOffsetX;
    float m_hitOffsetY;

    float m_speed;
    float m_gravity;
    float m_jumpForce;
    bool  m_facingRight;
    bool  m_onGround;

    // ----- State machine -----
    State m_state;
    int   m_hitsToEncase;
    int   m_hitsTaken;
    float m_stateTimer;              

    // ----- Rolling-state bookkeeping -----
    float m_rollOriginX;
    bool  m_rollDirectionRight;      
    float m_rollDistanceTravelled;   
    float m_rollSpeed;               

    virtual float getPartialEncaseDuration() const { return 2.0f; }

    bool m_oneHitEncase;

    // ----- Helpers -----
    void applyGravity(float dt);
    void syncHitBox();
    void syncSpritePositions();
    void jump();
    void updateStateTimers(float dt);
    virtual void applyStateSprite();
    void integrateRolling(float dt);   
    virtual void updateAnimation(float dt);

    static bool tryLoadTexture(sf::Texture& out, const std::string& path,
                               const char* tag);

    void loadEnemyAssets(
        const std::string& idlePath,
        const std::string& trappedPath,
        const std::string& unleash1Path,
        const std::string& unleash2Path,
        const std::string& unleash3Path,
        const std::string& snowEncase50Path,
        const std::string& snowEncase100Path,
        const std::string& snowEscape75Path,
        const std::string& snowEscape50Path,
        const std::string& snowEscape25Path
    );

    void loadAnimations(const std::string& walkBasePath, const std::string& jumpBasePath);

public:
    Enemy(sf::Vector2f pos,
          float spriteW, float spriteH,
          float hitW,    float hitH,
          float speed,
          int   hitsToEncase);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    virtual void updateAI(float dt) = 0;

    void setPosition(sf::Vector2f pos) override;
    void setOnGround(bool v) override { m_onGround = v; }

    virtual void takeAttackHit();
    void kickIntoRoll(bool facingRight);

    void setOneHitEncase(bool val) { m_oneHitEncase = val; }
    bool isOneHitEncase() const    { return m_oneHitEncase; }

    State getState()       const { return m_state; }
    bool  isOnGround()     const { return m_onGround; }
    bool  isFacingRight()  const { return m_facingRight; }
    int   getHitsTaken()   const { return m_hitsTaken; }
    bool  isRolling()      const { return m_state == State::Rolling; }
    bool  isSnowballed()   const { return m_state == State::Snowballed; }
};