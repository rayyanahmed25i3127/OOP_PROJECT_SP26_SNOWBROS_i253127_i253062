#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Baby enemy spawned by the Mogera boss (spec §6.3).
 *
 * Two-phase lifecycle:
 *   BALL phase   — travels in projectile arc (gravity affected) from Mogera's
 *                  mouth toward the left. Displays mogera_baby_ball.png.
 *                  CollisionDetector resolves platform landings and fires
 *                  setOnGround(true), which triggers the phase switch.
 *
 *   WALK phase   — walks left at WALK_SPEED px/s with 2-frame animation
 *                  (mogera_baby_walk_frame1/2.png, 0.25s each frame).
 *                  Gravity still applies so the CollisionDetector keeps
 *                  the baby pressed onto platform surfaces (same pattern
 *                  as Botom). Continues until it walks off-screen left.
 *
 * Key design rule: syncSpriteToPosition() is the SINGLE place that updates
 * both the sprite position AND the hitbox to match `position`. It is called
 * after every position change so sprite and hitbox can never drift apart.
 *
 * INHERITANCE: Entity → MogeraChild
 */
class MogeraChild : public Entity {
public:
    enum class Phase { Ball, Walking, Dead };

private:
    // --- Textures (persistent members — never local variables) ---
    sf::Texture m_ballTexture;
    sf::Texture m_walkTextures[2];
    bool        m_ballLoaded;
    bool        m_walkLoaded;

    // --- Sprite ---
    sf::Sprite  m_sprite;

    // --- Phase ---
    Phase m_phase;

    // --- Walk animation ---
    int   m_walkFrame;
    float m_walkFrameTimer;
    float m_landedY;               // Y position recorded at Ball→Walk transition;
                                   // used as a floor clamp in the walking phase
                                   // so babies don't sink without the collider.
    static constexpr float WALK_FRAME_TIME = 0.25f;
    static constexpr float WALK_SPEED      = 200.f;

    // --- Physics ---
    static constexpr float GRAVITY = 800.f;

    // --- Visual size ---
    float m_spriteW;
    float m_spriteH;

    // Single sync function — updates sprite texture, scale, position AND hitbox.
    // Must be called after every change to `position`, `m_phase`, or `m_walkFrame`.
    void syncSpriteToPosition();

public:
    explicit MogeraChild(sf::Vector2f pos, sf::Vector2f initialVelocity);

    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
    void setPosition(sf::Vector2f pos)       override;   // called by CollisionDetector
    void setOnGround(bool v)                 override;   // triggers Ball→Walk transition

    Phase getPhase()   const { return m_phase; }
    bool  isWalking()  const { return m_phase == Phase::Walking; }

    // Called by PlayState when a snowball hits this child.
    void takeHit();
};