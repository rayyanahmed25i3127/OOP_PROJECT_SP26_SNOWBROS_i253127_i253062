#pragma once
#include "Entity.hpp"

// player attack projectile 
class Projectile : public Entity 
{
protected:

    float m_gravity;

public:
    explicit Projectile(sf::Vector2f pos);
    virtual ~Projectile() = default;

};