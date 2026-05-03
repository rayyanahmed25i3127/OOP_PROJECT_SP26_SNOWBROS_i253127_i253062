#pragma once
#include "Entity.hpp"
#include "enemies/MogeraChild.hpp"
#include <SFML/Graphics.hpp>

// Yeh Level 5 ka boss Mogera hai jo ek hi jagah khara rehta hai aur hilta nahi. 
// Yeh har 6 seconds baad apna moun kholta hai aur teen MogeraChild 
// bahar phenkta hai. Isse khatam karne ke liye 50 snowball hits chahiye hote hain, 
// aur marne par yeh 120 gems (8 diamonds) aur 5000 score deta hai. Iska health bar 
// screen ke top-center par nazar aata hai aur yeh seedha Entity class se jurra hua hai.
class Mogera : public Entity {
public:
    enum class BossState {
        // states jisme yeh mogera ho skta hai
        Idle,           // wait tim between attacks
        OpenMouth1,     // mouth openframe 1
        OpenMouth2,     // mouth open and spawning babies
        Dying,          // death animation on the ground
        Dead            // fully removed
    };

    // thsi is linked with playstate file wo isko har dafa poll karta hai babies spawn karne k liay
    struct SpawnRequest {
        bool     pending;
        sf::Vector2f spawnPos;   // position of Mogera's mouth
    };

private:
//textures
    sf::Texture m_idleTexture;
    sf::Texture m_openMouth1Texture;
    sf::Texture m_openMouth2Texture;
    sf::Texture m_deathTexture;

    bool m_idleLoaded;
    bool m_open1Loaded;
    bool m_open2Loaded;
    bool m_deathLoaded;

    sf::Sprite m_sprite;

    //states
    BossState m_bossState;
    float     m_stateTimer;

    //timers
    static constexpr float IDLE_BEFORE_ATTACK = 4.5f;  // idle wait after throw
    static constexpr float IDLE_INTRO         = 0.5f;  // idle at start of cycle
    static constexpr float OPEN1_DURATION     = 0.5f;
    static constexpr float OPEN2_DURATION     = 1.0f;
    static constexpr float DEATH_DURATION     = 3.0f;

    //hp
    int  m_maxHits;
    int  m_hitsRemaining;
    bool m_canAttack;   // false once hitsRemaining == 0

    //child spawn signal 
    SpawnRequest m_spawnRequest;
    bool         m_babiesSpawnedThisCycle;   // prevents double-spawn in same open2 window

    
    float m_spriteW;
    float m_spriteH;

    //death fall
    static constexpr float DEATH_GRAVITY = 800.f;
    static constexpr float GROUND_Y      = 520.f;   // y where Mogera lands when dying

    //rewards
    bool  m_rewardPending;    // set true once, PlayState reads and clears

    void syncSprite();
    void syncHitBox();

public:
    // Mogera spawns at the lower-right platformt the rightmost lower platform sits at x=470, y=437 px
    explicit Mogera(sf::Vector2f pos);

    void update(float dt)               override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos)  override;

// it is called by PlayState when an attack ball hits Mogera.
    void takeSnowballHit();

    //getters
    BossState    getBossState()      const { return m_bossState;      }
    int          getHitsRemaining()  const { return m_hitsRemaining;  }
    int          getMaxHits()        const { return m_maxHits;        }
    bool         isDead()            const { return m_bossState == BossState::Dead; }
    bool         isDying()           const { return m_bossState == BossState::Dying; }
    bool         canDamagePlayer()   const { return m_canAttack;      }

    SpawnRequest getAndClearSpawnRequest() {
        SpawnRequest r = m_spawnRequest;
        m_spawnRequest.pending = false;
        return r;
    }

    // PlayState polls karke clear kardeta hai rewardsk liay
    bool getAndClearRewardPending() {
        bool r = m_rewardPending;
        m_rewardPending = false;
        return r;
    }
};