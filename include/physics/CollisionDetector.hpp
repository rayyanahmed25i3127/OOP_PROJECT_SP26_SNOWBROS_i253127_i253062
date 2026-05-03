#pragma once

#include "Entity.hpp"
#include "Platform.hpp"
#include "enemies/Enemy.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Resolves physics collisions between any Entity and the world,
 *        and detects player-enemy overlaps.
 *
 * Spec section 7.2: "All collisions go through CollisionDetector â€”
 * no ad-hoc overlap checks."
 *
 * Design: stateless utility class. Polymorphic over Entity (option b from
 * the design discussion): Player and Enemy both resolve through the same
 * code path â€” we never reach into subclass-specific members.
 *
 * Collision strategy for resolve():
 *   1. Screen-border wall clamp (left/right)
 *   2. Solid hit-box horizontal push-out
 *   3. One-way platform landing
 *   4. Fallback ground plane
 */
class CollisionDetector {
private:
    float m_leftWall;
    float m_rightWall;

public:
    CollisionDetector(float leftWall = 30.f, float rightWall = 770.f);

    /**
     * @brief Resolves all platform/wall collisions for an entity this frame.
     *
     * Call AFTER the entity's update() has moved it. Modifies position/velocity
     * via Entity setters and calls entity.setOnGround(bool) to report contact.
     */
    void resolve(Entity& entity,
                 Platform* const platforms[], int platformCount,
                 float prevX, float prevY) const;

    /**
     * @brief Checks whether the player's hit-box overlaps any living enemy.
     *
     * Per spec Â§7.2, this centralizes player-enemy overlap detection so
     * PlayState doesn't do ad-hoc AABB checks. Returns true on the first
     * overlap found. Ignores enemies in non-Alive state (snowballed
     * enemies can be pushed, not instantly lethal â€” matters when snowball
     * mechanic lands).
     *
     * @param player        The player to test.
     * @param enemies       Non-owning array of enemy pointers.
     * @param enemyCount    Number of valid entries in `enemies`.
     * @return true if player's hit-box intersects any alive enemy's hit-box.
     */
    bool checkEnemyContact(const Entity& player,
                           Enemy* const enemies[], int enemyCount) const;

    float getLeftWall()  const { return m_leftWall; }
    float getRightWall() const { return m_rightWall; }
};