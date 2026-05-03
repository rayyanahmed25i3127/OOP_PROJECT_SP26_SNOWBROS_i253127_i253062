#pragma once
#include "Entity.hpp"
#include "enemies/GamaBomb.hpp"
#include <SFML/Graphics.hpp>

/**
 * Gamakichi — final boss. Entity subclass (not Enemy — no encasement).
 *
 * Attack loop (repeats every 2.5s after previous bombs land):
 *   Idle (mouth closed)
 *   → OpenMouth (0.5s) — shows danger zones on 2 random platform X positions
 *   → Danger (2.0s)    — danger zone markers visible, player warned
 *   → Fire             — 2 GamaBombs launched at danger-zone targets
 *   → Wait for both bombs to finish exploding → back to Idle
 *
 * Death: health → 0 → sinks downward off screen over SINK_DURATION seconds.
 *
 * Drops: +250 gems on death.
 *
 * Inheritance: Entity → Gamakichi  (depth 2)
 */
class Gamakichi : public Entity {
public:
    enum class BossState { Idle, OpenMouth, Danger, Firing, Dying, Dead };

    struct RewardRequest { bool pending = false; };

    Gamakichi(sf::Vector2f pos);
    ~Gamakichi();

    void update(float dt);
    void draw(sf::RenderWindow& window);
    void drawHitBoxDebug(sf::RenderWindow& window, sf::Color c);

    void takeSnowballHit();

    bool isDying()         const { return m_bossState == BossState::Dying; }
    bool isDead()          const { return m_bossState == BossState::Dead;  }
    bool canDamagePlayer() const { return m_bossState != BossState::Dying
                                       && m_bossState != BossState::Dead;  }

    int  getHitsRemaining() const { return m_hitsRemaining; }
    int  getMaxHits()       const { return m_maxHits; }

    RewardRequest getAndClearRewardPending();

    // Danger-zone positions (world coords) for PlayState to draw markers
    static const int MAX_DANGER = 2;
    sf::Vector2f     m_dangerPos[MAX_DANGER];
    bool             m_dangerVisible;

private:
    // Sprites
    sf::Texture m_closedTex;  bool m_closedLoaded;
    sf::Texture m_openTex;    bool m_openLoaded;
    sf::Sprite  m_sprite;

    // Danger zone marker
    sf::Texture m_dangerTex;  bool m_dangerLoaded;
    sf::Sprite  m_dangerSprite;

    // State
    BossState m_bossState;
    float     m_stateTimer;
    int       m_hitsRemaining;
    int       m_maxHits;
    float     m_sinkOffset;   // for death animation

    // Bombs (max 2 per attack cycle)
    static const int MAX_BOMBS = 2;
    GamaBomb* m_bombs[MAX_BOMBS];
    int       m_bombCount;

    bool          m_rewardPending;

    // Attack timing
    static constexpr float OPEN_DURATION   = 0.5f;
    static constexpr float DANGER_DURATION = 2.0f;
    static constexpr float IDLE_DURATION   = 2.5f;
    static constexpr float SINK_DURATION   = 2.0f;

    // Platform surface positions to target (X centres, Y = surface)
    static const int NUM_TARGETS = 6;
    static const sf::Vector2f s_targets[NUM_TARGETS];

    void pickDangerZones();
    void fireBombs();
    void updateBombs(float dt);
    void drawBombs(sf::RenderWindow& window);
    void syncSprite();
};