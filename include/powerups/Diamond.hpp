#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

//these diamonds dropped on enemys double kill and with 15x diamond count
//its directly linked with entity class.
class Diamond : public Entity {
private:
    sf::Texture     m_texture;
    sf::Sprite      m_sprite;
    bool            m_textureLoaded;
    sf::CircleShape m_fallback;

    float m_lifetime;
    bool  m_isBonusDiamond;
    float m_gravity;
    float m_spriteWidth;
    float m_spriteHeight;
    float m_hitOffsetX;
    float m_hitOffsetY;

    void syncSprite();

public:
    static constexpr float DIAMOND_TIMEOUT = 8.0f;
    static constexpr int   GEM_VALUE       = 15;

    Diamond(sf::Vector2f pos);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;

    void setBonusDiamond(bool v) { m_isBonusDiamond = v; }
};