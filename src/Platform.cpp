#include "../include/Platform.h"

// Constructor
Platform::Platform(sf::Vector2f size, sf::Vector2f position)
{
    body.setSize(size);//size of dabba
    body.setFillColor(sf::Color::White);  //color of dabba 
    body.setOutlineThickness(0);//outline of dabba
    body.setOutlineColor(sf::Color::Green);//color of outline
    body.setPosition(position);//position of dabba (x,y)
}

// Draw function
void Platform::draw(sf::RenderWindow& window)
{
    window.draw(body);//draw platform on screen
}

// Return rectangle bounds (area of rectangle) -> (used in collision)
sf::FloatRect Platform::getBounds() const
{
    return body.getGlobalBounds();//actually this is the hitbox area
}

// Return position
//helpful:
//1) Objects ko allign karny ky liye(later)
//2)Spawning enemies

sf::Vector2f Platform::getPosition() const
{
    return body.getPosition();//only x,y of rectangle
}