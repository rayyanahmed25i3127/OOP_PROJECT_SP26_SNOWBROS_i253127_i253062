#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <string>

// powerup dropped on the odd double kills, add 50 score points and works for the player 
//InHERITANCE is smth like this abstract bases for Enemy / PowerUp / Projectile)
//Entity PowerUp.

class PowerUp : public Entity {
public:
    enum class Type {
        SpeedBoost,        //for 15s +50% movement speed
        SnowballPower,    
        DistanceIncrease,  // until level end and snowball travels full screen
        BalloonMode,       // 10s float upward, ground enemies can't harm
        Count_             // sentinel for random pick
    };

private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    sf::CircleShape m_fallback;   // shown if asset missing

    Type  m_type;
    float m_lifetime;             // seconds remaining before auto despawn
    float m_gravity;
    float m_spriteWidth;
    float m_spriteHeight;
    float m_hitOffsetX;
    float m_hitOffsetY;

    void syncSprite();

public:
    PowerUp(sf::Vector2f pos, Type type);

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setPosition(sf::Vector2f pos) override;
    void setOnGround(bool /*v*/) override {}   

    Type getType() const { return m_type; }

    static const char* typeName(Type t);
    static const char* texturePath(Type t);
};