#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief A platform the player stands on.
 *
 * Rendering: the platform owns a sf::Sprite that displays a texture
 * stretched to the platform's size. We use "Approach A" (stretched
 * whole-platform image) rather than per-tile repeating, which keeps
 * the code simple and matches the visual style we're going for.
 *
 * Because many platforms share the same texture, the texture itself
 * is passed in by const reference (non-owning) — Platform doesn't
 * load or own the texture, PlayState does. This avoids loading the
 * same .png file once per platform.
 *
 * Collision: getBounds() returns the platform's world rectangle.
 * The CollisionDetector (future work) uses this for hit-testing.
 */
class Platform {
private:
    sf::Sprite m_sprite;          // visual (stretched texture)
    sf::Vector2f m_position;
    sf::Vector2f m_size;

public:
    /**
     * @param texture  Reference to an already-loaded tile texture (non-owning).
     *                 PlayState loads the texture once and passes it in.
     * @param size     Platform width and height in pixels.
     * @param position Top-left corner of the platform in world coordinates.
     */
    Platform(const sf::Texture& texture, sf::Vector2f size, sf::Vector2f position);

    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    sf::Vector2f  getPosition() const { return m_position; }
    sf::Vector2f  getSize()     const { return m_size;     }
};