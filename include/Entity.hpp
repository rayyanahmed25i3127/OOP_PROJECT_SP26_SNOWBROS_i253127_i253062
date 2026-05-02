#pragma once
#include <SFML/Graphics.hpp>

/**
 * @brief Abstract base class for every physical, drawable game object.
 *
 * Owns position, velocity, a collision hit-box, and an alive flag.
 * Concrete subclasses (Player, Enemy, ...) implement update()/draw().
 *
 * Collision polymorphism:
 *   CollisionDetector operates on Entity& so Player, enemies, future
 *   projectiles, etc. all flow through the same resolve path. Entities
 *   that care about floor contact (Player, Enemy) override setOnGround();
 *   those that don't (future projectiles, pickups) inherit the no-op default.
 */
class Entity {
protected: // accessible to all child classes
    sf::Vector2f position;   // x,y world position
    sf::Vector2f velocity;   // per-second motion
    sf::FloatRect hitBox;    // AABB used by CollisionDetector
    bool alive = true;

public:
    explicit Entity(sf::Vector2f pos);
    virtual ~Entity() = default;

    // Pure virtual â€” every concrete Entity implements its own update/draw.
    // dt (delta-time) keeps motion framerate-independent across machines.
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    // Getters
    sf::Vector2f  getPosition() const { return position; }
    sf::Vector2f  getVelocity() const { return velocity; }
    sf::FloatRect getHitBox()   const { return hitBox;   }
    bool          isAlive()     const { return alive;    }

    // Setters â€” setPosition is virtual because subclasses must also
    // sync their sprite and hit-box offsets when externally repositioned.
    virtual void setPosition(sf::Vector2f pos) { position = pos; }
    void setVelocity(sf::Vector2f vel)         { velocity = vel; }
    void setAlive(bool a)                      { alive = a;      }

    // Ground-contact hook. CollisionDetector calls this after resolving
    // vertical collisions. Default no-op â€” only Player/Enemy override.
    virtual void setOnGround(bool /*onGround*/) {}

    // Debug: draws hit-box as a colored outline. Called from the state's
    // draw() when the F1/H debug toggle is on.
    void drawHitBoxDebug(sf::RenderWindow& window, sf::Color color);
};