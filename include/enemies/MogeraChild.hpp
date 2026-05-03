#pragma once
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

//Yeh mogera ka child hai or yeh do phases mein kaam karta hai... pehle yeh boss ke moun se ek ball 
//ki tarah nikal kar niche girta hai aur zameen se takrate hi chalne lagta hai. Chalne ke doran 
//yeh left side ki taraf jata hai aur is par gravity asar karti hai taake yeh platform se jra rahe. 
//Is mein sab se ahem rule yeh hai ke sprite aur hitbox hamesha ek sath update hote hain 
//taake game mein koi glitch na aaye, aur yeh seedha Entity class se hi relate hua hai.
class MogeraChild : public Entity {
public:
    enum class Phase { Ball, Walking, Dead };

private:
    //textures
    sf::Texture m_ballTexture;
    sf::Texture m_walkTextures[2];
    bool        m_ballLoaded;
    bool        m_walkLoaded;

    //sprote
    sf::Sprite  m_sprite;

//phases
    Phase m_phase;

//animation of walk
    int   m_walkFrame;
    float m_walkFrameTimer;
    float m_landedY;               
        
    static constexpr float WALK_FRAME_TIME = 0.25f;
    static constexpr float WALK_SPEED      = 200.f;

// its physics 
    static constexpr float GRAVITY = 800.f;

//size of enemy
    float m_spriteW;
    float m_spriteH;

   // updates evth about the sprite 
    void syncSpriteToPosition();

public:
    explicit MogeraChild(sf::Vector2f pos, sf::Vector2f initialVelocity);

    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
    void setPosition(sf::Vector2f pos)       override;   
    void setOnGround(bool v)                 override;   

    Phase getPhase()   const { return m_phase; }
    bool  isWalking()  const { return m_phase == Phase::Walking; }

    // it is called by playstate when a snowball hits this child sp that it will die 
    void takeHit(); // and btw there is no snowball encasement for this shi
};