#pragma once
#include <SFML/Graphics.hpp>

class Platform {
public:
    static const int MAX_HITBOXES = 4;

private:
    sf::Sprite    m_sprite;
    sf::Vector2f  m_position;
    sf::Vector2f  m_size;

    sf::FloatRect m_localBounds[MAX_HITBOXES];
    bool          m_solid[MAX_HITBOXES];   //when true then blocks from all sides
    int           m_hitboxCount;

public:
    Platform(const sf::Texture& texture, sf::Vector2f size, sf::Vector2f position);

    Platform(const sf::Texture& texture, sf::Vector2f size, sf::Vector2f position,
             const sf::FloatRect* localHitboxes, int count,
             const bool* solidFlags = nullptr);   // optional parameters

    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds(int i = 0) const;
    bool          isSolid(int i = 0) const;       
    int           getHitboxCount() const { return m_hitboxCount; }

    sf::Vector2f getPosition() const { return m_position; }
    sf::Vector2f getSize()     const { return m_size; }
};