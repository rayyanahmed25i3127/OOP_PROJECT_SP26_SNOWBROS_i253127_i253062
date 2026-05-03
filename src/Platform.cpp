#include "Platform.hpp"

Platform::Platform(const sf::Texture& texture, sf::Vector2f size, sf::Vector2f position)
    : m_sprite(texture), m_position(position), m_size(size), m_hitboxCount(1)
{
    auto texSize = texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        m_sprite.setScale({
            size.x / static_cast<float>(texSize.x),
            size.y / static_cast<float>(texSize.y)
        });
    }
    
    m_sprite.setPosition(position);

    const float insetX      = 15.f;
    const float insetTop    = 20.f;
    const float insetBottom = 10.f;
    m_localBounds[0].position = { insetX, insetTop };
    m_localBounds[0].size     = { size.x - 2.f * insetX,
                                  size.y - insetTop - insetBottom };
    m_solid[0] = false;   // default single-hitbox platforms are one-way
}

Platform::Platform(const sf::Texture& texture, sf::Vector2f size, sf::Vector2f position,
                   const sf::FloatRect* localHitboxes, int count,
                   const bool* solidFlags)
    : m_sprite(texture), m_position(position), m_size(size), m_hitboxCount(0)
{
    auto texSize = texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        m_sprite.setScale({
            size.x / static_cast<float>(texSize.x),
            size.y / static_cast<float>(texSize.y)
        });
    }
    m_sprite.setPosition(position);

    int n = count;
    if (n < 0) n = 0;
    if (n > MAX_HITBOXES) n = MAX_HITBOXES;
    for (int i = 0; i < n; ++i) {
        m_localBounds[i] = localHitboxes[i];
        m_solid[i] = (solidFlags ? solidFlags[i] : false);
    }
    m_hitboxCount = n;
}

void Platform::draw(sf::RenderWindow& window) {
    window.draw(m_sprite);
}

sf::FloatRect Platform::getBounds(int i) const {
    sf::FloatRect b;
    if (i < 0 || i >= m_hitboxCount) {
        b.position = m_position;
        b.size     = { 0.f, 0.f };
        return b;
    }
    b.position = { m_position.x + m_localBounds[i].position.x,
                   m_position.y + m_localBounds[i].position.y };
    b.size     = m_localBounds[i].size;
    return b;
}

bool Platform::isSolid(int i) const {
    if (i < 0 || i >= m_hitboxCount) return false;
    return m_solid[i];
}