#include "Platform.hpp"

Platform::Platform(const sf::Texture& texture, sf::Vector2f size, sf::Vector2f position)
    : m_sprite(texture)        // SFML 3 requires texture on sprite construction
    , m_position(position)
    , m_size(size)
{
    // Stretch the sprite to exactly the platform's size.
    // getSize() returns sf::Vector2u (unsigned), so we cast to float.
    auto texSize = texture.getSize();
    float scaleX = size.x / static_cast<float>(texSize.x);
    float scaleY = size.y / static_cast<float>(texSize.y);

    m_sprite.setScale({scaleX, scaleY});
    m_sprite.setPosition(position);
}

void Platform::draw(sf::RenderWindow& window) {
    window.draw(m_sprite);
}

sf::FloatRect Platform::getBounds() const {
    return sf::FloatRect(m_position, m_size);
}