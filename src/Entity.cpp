#include "Entity.h"

Entity::Entity(sf::Vector2f pos) : position(pos), velocity(0.f, 0.f), alive(true) {
    hitBox.position = pos;
    hitBox.size     = { 40.f, 40.f }; // default; subclasses can override (change)
}

void Entity::drawHitBoxDebug(sf::RenderWindow& window, sf::Color color) {
    sf::RectangleShape box(sf::Vector2f(hitBox.size.x, hitBox.size.y));//create rectangle isze same as hitbox
    box.setPosition(sf::Vector2f(hitBox.position.x, hitBox.position.y));//position of rectangle at entity
    box.setFillColor(sf::Color::Transparent);//We are going to use different colors for debugging
    box.setOutlineColor(color);
   box.setOutlineThickness(1.f);
    window.draw(box);//draw this on screen
}