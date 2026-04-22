#pragma once
#include <SFML/Graphics.hpp>

class Platform {
private:
    sf::RectangleShape body;   //platform object for drawing and displaying

public:
    // Constructor
    Platform(sf::Vector2f size, sf::Vector2f position);

    // Draw platform
    void draw(sf::RenderWindow& window);

    // Get bounds for collision
    sf::FloatRect getBounds() const;

    // Get position 
    sf::Vector2f getPosition() const;
};