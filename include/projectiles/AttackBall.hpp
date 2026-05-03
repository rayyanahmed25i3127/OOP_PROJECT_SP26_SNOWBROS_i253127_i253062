#pragma once
#include "projectiles/Projectile.hpp"
#include <string>

// player attack, moves in a straight direction horizontally in 220px per sec speed and its sused to kill enemies...
class AttackBall : public Projectile {
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    sf::CircleShape m_fallbackShape;  // if sprite fails to load

    float m_distanceTravelled;
    float m_maxDistance;
    float m_speed;
    bool  m_facingRight;

    //power up enabled, then distance increased
    bool  m_maxRangeMode;      // when true m_maxDistance = 800 screen ki total width 

    float m_spriteWidth;
    float m_spriteHeight;

public:
    AttackBall(sf::Vector2f pos, bool facingRight,
               const std::string& texturePath = "assets/sprites/attackball_blue.png");

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;

    bool isFacingRight() const { return m_facingRight; }


    void setMaxRangeMode(bool val) { m_maxRangeMode = val; }
    bool isMaxRangeMode() const    { return m_maxRangeMode; }
};