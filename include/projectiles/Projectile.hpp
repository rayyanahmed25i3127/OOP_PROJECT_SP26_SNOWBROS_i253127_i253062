#pragma once
#include "Entity.hpp"

/**
 * @brief Abstract base for all projectiles (spec Â§11.1 requires this).
 *
 * A Projectile is an Entity that:
 *   - Moves in a straight-ish line under its own velocity
 *   - Has finite lifetime (distance-based, time-based, or on-impact)
 *   - Flags itself dead (alive=false) when it's done, so the owning
 *     state can garbage-collect it from its projectile array.
 *
 * Concrete subclasses: Snowball, (future) Knife, ArtilleryBlast.
 *
 * INHERITANCE HIERARCHY (spec Â§11.1 depth requirement):
 *   Entity -> Projectile -> Snowball
 */
class Projectile : public Entity {
protected:
    // Projectiles don't normally care about gravity, but Enemy subclasses
    // might want arcing projectiles later. Base stores gravity so subclasses
    // can opt in by setting a non-zero value.
    float m_gravity;

public:
    explicit Projectile(sf::Vector2f pos);
    virtual ~Projectile() = default;

    // All projectiles still need update/draw; subclasses implement them.
    // Projectile does NOT implement Template Method here because Snowball
    // wraps around screens (odd motion) while a Knife flies in a straight
    // line â€” too divergent to share a skeleton at this layer.
};