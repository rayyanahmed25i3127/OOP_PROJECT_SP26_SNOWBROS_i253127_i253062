#pragma once
#include <SFML/Graphics.hpp>

class Entity {
protected://can be used by all child classes
    sf::Vector2f position;//x,y for position
    sf::Vector2f velocity;//2 for velocity
    sf::FloatRect hitBox;
    bool alive = true;

public:
    explicit Entity(sf::Vector2f pos);
    virtual ~Entity() = default;//destructor

    //virtual functions because child classes will define pure logic:
    virtual void update(float dt) = 0;//update movements, position,speed with dt
    //dt is to make sure that game flows better in all PCs
    virtual void draw(sf::RenderWindow& window) = 0;//draw that update on screen

    // Getters
    sf::Vector2f  getPosition()  const {
         return position; }
    sf::Vector2f  getVelocity()  const {
         return velocity; }
    sf::FloatRect getHitBox()    const {
         return hitBox;   }
    bool          isAlive()      const { 
        return alive;    }

    // Setters
    virtual void setPosition(sf::Vector2f pos) { position = pos; }
    void setVelocity(sf::Vector2f vel) { 
        velocity = vel; }
    void setAlive(bool a)              {
         alive = a;      }
    //This function is to see collisions.
    // Debug: draws hitbox as colored outline (call only when F1 is pressed)
    void drawHitBoxDebug(sf::RenderWindow& window, sf::Color color);
};