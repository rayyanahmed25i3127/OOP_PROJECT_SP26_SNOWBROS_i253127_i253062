#pragma once
#include "enemies/FlyngFoogaFoog.hpp"

//Yeh bhai apni urrne ki speed ko kabhi kam aur kabhi zyada karta rehta hai aur thori thori 
//der baad player ki taraf knives phenkta hai... Isse snowball encase karne ke 
//liye 3 snowball hits ki zaroorat hoti hai aur iski apni mukhtalif animations (walk, fly, throw wagera)
//hain. Game ki PlayState har frame check karti hai ke kab naya knife spawn karna hai taake enemy ka attack continuous rahe.
class Tornado : public FlyngFoogaFoog {
public:
    struct KnifeRequest {
        bool         pending  = false;
        sf::Vector2f spawnPos;
        sf::Vector2f direction;
    };

private:
    // Walk animation
    sf::Texture m_walkTextures[3]; bool m_walkTexLoaded;
    int   m_walkTexFrame; float m_walkTexTimer;

   //flight frames
    sf::Texture m_flyFrames[2]; bool m_flyFramesLoaded;
    int m_flyFrame; float m_flyFrameTimer;

    // directional flips and jump frames
    sf::Texture m_leftTex;  bool m_leftLoaded;
    sf::Texture m_rightTex; bool m_rightLoaded;
    sf::Texture m_jumpTex2; bool m_jumpLoaded2;

    //turnanim
    sf::Texture m_turnFrames[4]; bool m_turnLoaded;
    bool  m_isTurning; int m_turnFrame; float m_turnTimer;
    bool  m_prevFacingRight;

    //throw knife anim
    sf::Texture m_throwFrames[2]; bool m_throwLoaded;
    bool  m_isThrowing; int m_throwFrame; float m_throwTimer;
    bool  m_knifeSpawnedThisCycle;

    //encapsulation textures
    sf::Texture m_enc25Tex; bool m_enc25Loaded;
    sf::Texture m_enc50Tex; bool m_enc50Loaded;

    //knife, its secondary attack
    float        m_knifeTimer;
    float        m_knifeCooldown;
    KnifeRequest m_knifeRequest;

    //flying speed is random bcuz it teleports 
    float m_flySpeed;

    static constexpr float FLY_FRAME_TIME   = 0.15f;
    static constexpr float TURN_FRAME_TIME  = 0.08f;
    static constexpr float THROW_FRAME_TIME = 0.15f;
    static constexpr float WALK_TEX_TIME    = 0.25f;
    static constexpr float MIN_FLY_SPEED    = 60.f;
    static constexpr float MAX_FLY_SPEED    = 280.f;
    static constexpr float MIN_KNIFE_CD     = 2.5f;
    static constexpr float MAX_KNIFE_CD     = 5.5f;
    static constexpr float PARTIAL_DURATION = 1.5f;  // replaces private fooga version

    float getPartialEncaseDuration() const override { return PARTIAL_DURATION; }

    void randomizeFlySpeed();
    void randomizeKnifeCooldown();

    void updateAnimation(float dt) override;
    void applyStateSprite()        override;
    void updateStateTimers(float dt) override;

public:
    explicit Tornado(sf::Vector2f pos);

    void update(float dt) override;
    void takeAttackHit() override;

    KnifeRequest getAndClearKnifeSpawn();
};