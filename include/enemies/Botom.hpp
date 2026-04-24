#pragma once
#include "enemies/Enemy.hpp"

/**
 * @brief The basic ground-walking enemy (spec §6.2.1).
 *
 * Behavior:
 *   - Walks horizontally at a constant speed.
 *   - At random intervals (1-3 seconds), flips direction. Creates the
 *     "natural, pseudo-random" movement feel the spec calls for.
 *   - At random intervals (3-6 seconds), jumps straight up. Keeps
 *     horizontal velocity, so often ends up on a higher platform.
 *     This is pseudo-random — not path-planning — preserving spec
 *     compliance while adding vertical mobility.
 *   - Turns around on wall contact (CollisionDetector zeroes velocity.x
 *     against screen borders; we detect that and flip facing).
 *   - Gravity pulls it downward; platforms and fallback ground catch it
 *     via the shared CollisionDetector.
 *   - Falls off platform edges freely.
 *
 * Encasing (spec §6.2.1): "Encased by 1-2 direct snowball hits." Using 2.
 *
 * Inheritance role: Botom is the base of the flying-enemy chain —
 *   Enemy -> Botom -> FlyngFoogaFoog -> Tornado   (viva: depth 5)
 */
class Botom : public Enemy {
private:
    // Direction-flip timer — counts down, flips direction on zero.
    float m_directionTimer;
    float m_minDirectionInterval;
    float m_maxDirectionInterval;

    // Jump timer — counts down, triggers jump() on zero (if grounded).
    float m_jumpTimer;
    float m_minJumpInterval;
    float m_maxJumpInterval;

    // Last frame's walk velocity — non-zero to zero transition means we
    // hit a wall, flip direction.
    float m_lastWalkVelocityX;

    void rollDirectionTimer();
    void rollJumpTimer();

public:
    explicit Botom(sf::Vector2f pos);

    void updateAI(float dt) override;
};