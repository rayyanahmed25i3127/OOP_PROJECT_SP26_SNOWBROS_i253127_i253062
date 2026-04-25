#include "projectiles/Projectile.hpp"

Projectile::Projectile(sf::Vector2f pos)
    : Entity(pos)
    , m_gravity(0.f)
{}