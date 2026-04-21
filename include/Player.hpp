#pragma once
#include "Entity.h"
#include <SFML/Graphics.hpp>

class Player : public Entity {//child class from entity
    //player already has position,velocity,hitbox and alive
private://for the player only.
    sf::RectangleShape body;//this is twmporary visual for player (blue box)

    float speed; //horizontal movement
    float jumpForce;//Upward movement
    float gravity;//pulls down

    bool onGround;

public:
    Player(sf::Vector2f pos);//takes initial position

    void handleInput();//keabord reading 
    void applyGravity(float dt);//downward force with dt alrd explained

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;//player draws itself
};