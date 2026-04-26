#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Abstract base for every enemy in the game.
 *
 * Responsibilities beyond Entity:
 *   - AI-driven movement (pure virtual updateAI)
 *   - Encase state machine (Snow Bros lifecycle)
 *   - Facing direction & sprite flipping
 *   - Ground contact (gravity walkers)
 *   - Jump helper
 *   - COMPOSITE rendering: body sprite + snow overlay sprite layered
 *
 * DESIGN PATTERN — Template Method:
 *   update(dt) is concrete. Order:
 *       1. applyGravity(dt)
 *       2. updateStateTimers(dt)   — advances the state machine
 *       3. updateAI(dt)            — pure virtual hook (skipped when frozen)
 *       4. integrate velocity
 *       5. sync hit-box & sprite positions
 *
 * SNOW BROS LIFECYCLE (spec §7.1):
 *
 *   Alive(0 hits)                         ← walking, LETHAL
 *     ├─ (1 hit)       → PartialEncase    ← frozen, not lethal, 2s timer
 *     │                   ├─ (hit 2)     → Snowballed
 *     │                   └─ (2s timer)  → Alive(0)    (shakes partial off)
 *     │
 *     └─ (2 hits direct, impossible — covered above)
 *
 *   Snowballed (100%)                     ← frozen, not lethal, 3s timer,
 *                                           KICKABLE (auto on walk-in)
 *     ├─ (player contact) → Rolling
 *     └─ (3s timer)       → Escaping75 → 50 → 25 → Alive  (1s each)
 *
 *   Escaping75 / 50 / 25                  ← frozen, NOT lethal, NOT kickable
 *     └─ (1s each)        → next stage / Alive
 *
 *   Rolling                               ← moves fast, wraps once, dies at
 *                                           kick-origin or on Alive-enemy kill
 *     ├─ (hits Alive enemy)      → that enemy → Dead; chain counter ++
 *     ├─ (returns to kick-origin) → this rolling enemy → Dead
 *     └─ (falls off screen)       → Dead
 *
 * COMPOSITE RENDERING:
 *   Body pose × snow overlay, layered. See applyStateSprite().
 *
 * INHERITANCE (spec §15.2 depth ≥ 3):
 *   Entity → Enemy → Botom → FlyngFoogaFoog → Tornado   (depth 5)
 *
 * POWER-UP: Snowball Power (Phase 4.5)
 *   - When active, m_oneHitEncase = true
 *   - Enemies are encased in 1 hit instead of 2
 */
class Enemy : public Entity {
public:
    enum class State {
        Alive,
        PartialEncase,     // NEW: 1 hit taken, frozen, waiting for 2nd hit or timer
        Snowballed,        // fully encased, frozen, kickable
        Escaping75,
        Escaping50,
        Escaping25,
        Rolling,           // kicked — moves fast, kills Alive enemies
        Dead
    };

protected:
    // ----- BODY textures (multi-pose) -----
    sf::Texture m_idleTexture;
    sf::Texture m_trappedTexture;
    sf::Texture m_unleash1Texture;   // used for Escaping75
    sf::Texture m_unleash2Texture;   // used for Escaping50
    sf::Texture m_unleash3Texture;   // used for Escaping25

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
    float m_stateTimer;              // countdown for current state

    // ----- Rolling-state bookkeeping -----
    // Origin where the rolling started — snowball dies when it wraps back here.
    float m_rollOriginX;
    bool  m_rollDirectionRight;      // which way it's rolling
    float m_rollDistanceTravelled;   // absolute distance moved so far
    float m_rollSpeed;               // 600-700 per design choice

    // Per-enemy escape-stage duration. Botom uses PARTIAL_DURATION=2s;
    // tougher enemies can override to 1s per stage (declared virtual for
    // future subclasses).
    virtual float getPartialEncaseDuration() const { return 2.0f; }

    // ===== POWER-UP: Snowball Power =====
    // When true, enemy is encased in 1 hit instead of 2
    bool m_oneHitEncase;

    // ----- Helpers -----
    void applyGravity(float dt);
    void syncHitBox();
    void syncSpritePositions();
    void jump();
    void updateStateTimers(float dt);
    void applyStateSprite();
    void integrateRolling(float dt);   // movement logic for Rolling state

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

    // Register an attack-ball hit. Accepted in Alive and PartialEncase states.
    virtual void takeAttackHit();

    // Player kicked this enemy — transitions Snowballed → Rolling.
    // Ignored if state != Snowballed.
    void kickIntoRoll(bool facingRight);

    // Called by PlayState when a Rolling enemy kills another Alive enemy.
    // Doesn't do anything here — kill/score accounting lives in PlayState —
    // but centralizes the "did we just do a chain kill" concept.

    // ===== POWER-UP: Snowball Power =====
    /**
     * @brief Set whether this enemy is encased in 1 hit (true) or 2 hits (false).
     *
     * Called when Snowball Power power-up is activated/deactivated.
     */
    void setOneHitEncase(bool val) { m_oneHitEncase = val; }
    bool isOneHitEncase() const    { return m_oneHitEncase; }

    // Getters
    State getState()       const { return m_state; }
    bool  isOnGround()     const { return m_onGround; }
    bool  isFacingRight()  const { return m_facingRight; }
    int   getHitsTaken()   const { return m_hitsTaken; }
    bool  isRolling()      const { return m_state == State::Rolling; }
    bool  isSnowballed()   const { return m_state == State::Snowballed; }
};