#pragma once
#include "Entity.hpp"
#include "enemies/GamaBomb.hpp"
#include <SFML/Graphics.hpp>

// Yeh boss aam enemies se mukhtalif hai kyunke isse encase
// nahi kiya ja sakta aur yeh har 2.5 seconds baad hamla karta hai. 
// Hamle ke doran yeh do random jaghon par "danger zones" dikhata hai aur phir 
// wahan GamaBombs fire karta hai. Jab iski health khatam ho jati hai toh yeh 
// screen se niche gir jata hai aur sath mein 250 gems chorta hai. Iska 
// inheritance seedha Entity class se hai kyunke iske features aam enemies se bohot alag hain.
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

   //danger zones ki position
    static const int MAX_DANGER = 2;
    sf::Vector2f     m_dangerPos[MAX_DANGER];
    bool             m_dangerVisible;

private:
//sprites
    sf::Texture m_closedTex;  bool m_closedLoaded;
    sf::Texture m_openTex;    bool m_openLoaded;
    sf::Sprite  m_sprite;

// ye actual danger zone banata hai
    sf::Texture m_dangerTex;  bool m_dangerLoaded;
    sf::Sprite  m_dangerSprite;

//state in the the bass actually is
    BossState m_bossState;
    float     m_stateTimer;
    int       m_hitsRemaining;
    int       m_maxHits;
    float     m_sinkOffset;   // for death animation

    //bombs - max2 
    static const int MAX_BOMBS = 2;
    GamaBomb* m_bombs[MAX_BOMBS];
    int       m_bombCount;

    bool          m_rewardPending;

    //attack timing
    static constexpr float OPEN_DURATION   = 0.5f;
    static constexpr float DANGER_DURATION = 2.0f;
    static constexpr float IDLE_DURATION   = 2.5f;
    static constexpr float SINK_DURATION   = 2.0f;

    // platform surface positions to target (X centres, Y = surface)
    static const int NUM_TARGETS = 6;
    static const sf::Vector2f s_targets[NUM_TARGETS];

    void pickDangerZones();
    void fireBombs();
    void updateBombs(float dt);
    void drawBombs(sf::RenderWindow& window);
    void syncSprite();
};