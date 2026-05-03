#pragma once
#include <SFML/Graphics.hpp>

//ye base class hai jo har object ki position, speed/velocity, aur hitbox
//ka track rakhti hai. Iska sab se bara faida yeh hai ke CollisionDetector tamam cheezon, chahe
//woh player ho ya enemy, dono par par ek hi tarah se kaam karta hai kyunke woh sab isi "Entity" se linked hain. 
//Jo objects zameen par kharay hote hain woh setOnGround() ka istemal karte hain, jabke projectiles 
//like knives, bombs etc ise ignore kar dete hain.
class Entity {
protected: // accessible to all child classes
    sf::Vector2f position;   //coordinating pos
    sf::Vector2f velocity;   //speed per sec
    sf::FloatRect hitBox;    // hiiting detect karne k liay 
    bool alive = true;

public:
    explicit Entity(sf::Vector2f pos);
    virtual ~Entity() = default;

    
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    //getters
    sf::Vector2f  getPosition() const { return position; }
    sf::Vector2f  getVelocity() const { return velocity; }
    sf::FloatRect getHitBox()   const { return hitBox;   }
    bool          isAlive()     const { return alive;    }

    //setters
    virtual void setPosition(sf::Vector2f pos) { position = pos; }
    void setVelocity(sf::Vector2f vel)         { velocity = vel; }
    void setAlive(bool a)                      { alive = a;      }

    
    virtual void setOnGround(bool /*onGround*/) {}

    // draws colorful hot boxes outline when h or f1 pressed
    void drawHitBoxDebug(sf::RenderWindow& window, sf::Color color);
};