#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Abstract base for every enemy in the game.
 *
 * Inherits from Entity and extends it with enemy-specific concerns:
 *   - AI-driven movement (per-subclass, implemented via pure virtual updateAI)
 *   - Health / encasing state (how many snowball hits to defeat)
 *   - Facing direction (for sprite flipping)
 *   - Ground-contact awareness (so gravity-affected walkers can stop on platforms)
 *   - Jumping (shared helper — any enemy that has feet can jump)
 *
 * DESIGN PATTERN — Template Method:
 *   update(dt) is CONCRETE and shared by all enemies. It does, in order:
 *       1. applyGravity(dt)
 *       2. updateAI(dt)              <-- pure virtual, varies per enemy
 *       3. position += velocity * dt
 *       4. syncHitBox()
 *   Subclasses (Botom, FlyngFoogaFoog, Tornado, Mogera, Gamakichi) only
 *   override updateAI(). This prevents copy-pasting physics across every
 *   enemy class.
 *
 * INHERITANCE HIERARCHY (spec §15.2 requires depth >= 3):
 *   Entity -> Enemy -> Botom -> FlyngFoogaFoog -> Tornado   (depth 5)
 *
 * SNOW BROS LIFECYCLE (spec §7.1):
 *   Alive -> (hit by snowballs) -> Snowballed -> (kicked) -> Rolling -> Dead
 */
class Enemy : public Entity {
public:
    enum class State { Alive, Snowballed, Rolling, Dead };

protected:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;

    float m_spriteWidth;
    float m_spriteHeight;

    float m_hitOffsetX;
    float m_hitOffsetY;

    float m_speed;          // horizontal walking speed (pixels/sec)
    float m_gravity;        // downward acceleration (pixels/sec^2)
    float m_jumpForce;      // upward impulse on jump (pixels/sec, negative)
    bool  m_facingRight;
    bool  m_onGround;

    State m_state;
    int   m_hitsToEncase;
    int   m_hitsTaken;

    // Shared helpers for subclasses
    void applyGravity(float dt);
    void syncHitBox();
    void syncSpritePosition();

    // Trigger a jump if the enemy is grounded. Subclasses call this from
    // updateAI(). No-op if already airborne — mirrors Player's jump logic.
    void jump();

public:
    Enemy(sf::Vector2f pos,
          float spriteW, float spriteH,
          float hitW,    float hitH,
          float speed,
          int   hitsToEncase);

    // Template method — do NOT override in subclasses. Override updateAI instead.
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Pure virtual — each concrete enemy defines its own AI behavior.
    virtual void updateAI(float dt) = 0;

    // Entity overrides
    void setPosition(sf::Vector2f pos) override;
    void setOnGround(bool v) override { m_onGround = v; }

    // Snow Bros combat hook (stub for now — snowball task fills this in)
    virtual void takeSnowballHit();

    // Getters
    State getState()      const { return m_state; }
    bool  isOnGround()    const { return m_onGround; }
    bool  isFacingRight() const { return m_facingRight; }
};