#pragma once
#include "Entity.hpp"
#include "enemies/MogeraChild.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Boss enemy for Level 5 (spec §6.3).
 *
 * Mogera is a large stationary boss that:
 *   - Sits on the lower-right platform at all times (except death).
 *   - Cycles through an animation loop every ~6 seconds:
 *       idle (0.5s) → open_mouth_frame1 (0.5s) → open_mouth_frame2 (1.0s, throws babies) → idle (4.5s) → repeat
 *   - Throws 3 MogeraChild enemies when open_mouth_frame2 begins.
 *   - Takes 30 snowball hits to die.
 *   - On death: falls to the ground, displays death frame for 3s then disappears.
 *   - Drops 120 gems (8 × diamond icons) + 5000 score on death.
 *   - After health runs out, can no longer spawn children or damage player.
 *
 * Boss health bar rendered externally by PlayState (top-center, below level indicator).
 *
 * INHERITANCE: Entity → Mogera   (depth 2 from Entity)
 */
class Mogera : public Entity {
public:
    enum class BossState {
        Idle,           // waiting between attacks
        OpenMouth1,     // mouth opening, frame 1
        OpenMouth2,     // mouth open, spawning babies
        Dying,          // death animation on the ground
        Dead            // fully removed
    };

    // Signal struct — PlayState polls this every frame to spawn babies
    struct SpawnRequest {
        bool     pending;
        sf::Vector2f spawnPos;   // position of Mogera's mouth
    };

private:
    // --- Textures ---
    sf::Texture m_idleTexture;
    sf::Texture m_openMouth1Texture;
    sf::Texture m_openMouth2Texture;
    sf::Texture m_deathTexture;

    bool m_idleLoaded;
    bool m_open1Loaded;
    bool m_open2Loaded;
    bool m_deathLoaded;

    sf::Sprite m_sprite;

    // --- State machine ---
    BossState m_bossState;
    float     m_stateTimer;

    // Timing constants
    static constexpr float IDLE_BEFORE_ATTACK = 4.5f;  // idle wait after throw
    static constexpr float IDLE_INTRO         = 0.5f;  // idle at start of cycle
    static constexpr float OPEN1_DURATION     = 0.5f;
    static constexpr float OPEN2_DURATION     = 1.0f;
    static constexpr float DEATH_DURATION     = 3.0f;

    // --- Health ---
    int  m_maxHits;
    int  m_hitsRemaining;
    bool m_canAttack;   // false once hitsRemaining == 0

    // --- Baby spawn signal ---
    SpawnRequest m_spawnRequest;
    bool         m_babiesSpawnedThisCycle;   // prevents double-spawn in same open2 window

    // --- Visual ---
    float m_spriteW;
    float m_spriteH;

    // --- Death fall ---
    static constexpr float DEATH_GRAVITY = 800.f;
    static constexpr float GROUND_Y      = 520.f;   // y where Mogera lands when dying

    // --- Reward flags ---
    bool  m_rewardPending;    // set true once — PlayState reads and clears

    void syncSprite();
    void syncHitBox();

public:
    // Mogera spawns at the lower-right platform.
    // The rightmost lower platform sits at x≈470, y=437. 
    // We position Mogera flush with the right wall at y≈355 (platform top - sprite height).
    explicit Mogera(sf::Vector2f pos);

    void update(float dt)               override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos)  override;

    // Called by PlayState when an attack ball hits Mogera.
    void takeSnowballHit();

    // Getters for PlayState
    BossState    getBossState()      const { return m_bossState;      }
    int          getHitsRemaining()  const { return m_hitsRemaining;  }
    int          getMaxHits()        const { return m_maxHits;        }
    bool         isDead()            const { return m_bossState == BossState::Dead; }
    bool         isDying()           const { return m_bossState == BossState::Dying; }
    bool         canDamagePlayer()   const { return m_canAttack;      }

    // PlayState polls & clears this each frame
    SpawnRequest getAndClearSpawnRequest() {
        SpawnRequest r = m_spawnRequest;
        m_spawnRequest.pending = false;
        return r;
    }

    // PlayState polls & clears once for reward
    bool getAndClearRewardPending() {
        bool r = m_rewardPending;
        m_rewardPending = false;
        return r;
    }
};