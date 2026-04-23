#pragma once

#include "Player.hpp"
#include "Platform.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Resolves physics collisions between the player and the world.
 *
 * Spec section 7.2: "All collisions go through CollisionDetector —
 * no ad-hoc overlap checks."
 *
 * Design: stateless utility class. It doesn't own any game objects;
 * it just provides collision resolution services called once per frame
 * by PlayState after Player::update() has moved the player.
 *
 * Collision strategy:
 *   - Swept AABB (Axis-Aligned Bounding Box) resolution, per-axis
 *   - Horizontal first, then vertical (prevents corner-catch bugs)
 *   - One-way platforms: only block falling player from above
 *   - Screen borders act as solid walls at configurable X coordinates
 *
 * Usage per frame (inside PlayState::update):
 *   1. Player::update(dt) — applies velocity, gravity, moves position
 *   2. collider.resolve(player, platforms, platformCount)
 */
class CollisionDetector {
private:
    // World boundaries (the icy border walls in bg_lvl1.png).
    // Player's hitbox is clamped to stay within [m_leftWall, m_rightWall].
    float m_leftWall;
    float m_rightWall;

public:
    /**
     * @param leftWall   X coord where the left ice wall ends (player min X)
     * @param rightWall  X coord where the right ice wall starts (player max X)
     */
    CollisionDetector(float leftWall = 30.f, float rightWall = 770.f);

    /**
     * @brief Resolves all collisions for the player this frame.
     *
     * Call AFTER the player's update() has moved them. This method:
     *   1. Clamps the player horizontally against the screen walls
     *   2. For each platform, checks if the player landed on top and snaps
     *      them to the platform surface if so (setting onGround)
     *
     * @param player        The player to resolve. Position and velocity may be modified.
     * @param platforms     Array of platform pointers (non-owning).
     * @param platformCount Number of valid entries in the platforms array.
     * @param prevY         Player's Y position BEFORE this frame's movement
     *                      (needed to distinguish jumping-through from landing-on).
     */
    void resolve(Player& player,
                 Platform* const platforms[],
                 int platformCount,
                 float prevY) const;

    // Getters for the walls (debug display / future enemy logic)
    float getLeftWall()  const { return m_leftWall; }
    float getRightWall() const { return m_rightWall; }
};