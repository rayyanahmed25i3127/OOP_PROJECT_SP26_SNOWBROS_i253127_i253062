#pragma once
#include "enemies/FlyngFoogaFoog.hpp"

/**
 * Tornado — inherits FlyngFoogaFoog, adds:
 *   - Randomized fly speed (slow ↔ fast)
 *   - Knife throwing toward player at intervals
 *   - 3 hits to encase (tornado_encased25/50 → snowball)
 *   - Own walk/fly/throw/turn/jump animations
 *
 * PlayState polls getAndClearKnifeSpawn() each frame to spawn Knife projectiles.
 *
 * Inheritance: Entity → Enemy → FlyngFoogaFoog → Tornado  (depth 4)
 */
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

    // Fly frames (tornado_frame1/2)
    sf::Texture m_flyFrames[2]; bool m_flyFramesLoaded;
    int m_flyFrame; float m_flyFrameTimer;

    // Directional / jump
    sf::Texture m_leftTex;  bool m_leftLoaded;
    sf::Texture m_rightTex; bool m_rightLoaded;
    sf::Texture m_jumpTex2; bool m_jumpLoaded2;

    // Turn animation (4 frames)
    sf::Texture m_turnFrames[4]; bool m_turnLoaded;
    bool  m_isTurning; int m_turnFrame; float m_turnTimer;
    bool  m_prevFacingRight;

    // Throw animation (2 frames)
    sf::Texture m_throwFrames[2]; bool m_throwLoaded;
    bool  m_isThrowing; int m_throwFrame; float m_throwTimer;
    bool  m_knifeSpawnedThisCycle;

    // Encasement textures (3-hit)
    sf::Texture m_enc25Tex; bool m_enc25Loaded;
    sf::Texture m_enc50Tex; bool m_enc50Loaded;

    // Knife
    float        m_knifeTimer;
    float        m_knifeCooldown;
    KnifeRequest m_knifeRequest;

    // Randomized fly speed (overrides fooga's 100 px/s)
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