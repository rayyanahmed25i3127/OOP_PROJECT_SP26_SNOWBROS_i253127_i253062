#pragma once
#include "enemies/Enemy.hpp"

// sab same as botom red, just color change, appearance change and speed increase and some more hp
class BotomBlue : public Enemy {
private:
    // extra partial encasement texture (75% stage) - kyunke hp zada hai
    sf::Texture m_encased25Texture;   
    sf::Texture m_encased50Texture;   
    bool m_encased25Loaded;
    bool m_encased50Loaded;



// direction / flip / jump sab kuch botm red wala hi
    float m_directionTimer;
    float m_jumpTimer;
    float m_lastWalkVelocityX;

    void rollDirectionTimer();
    void rollJumpTimer();

    static float randomBetween(float lo, float hi);

protected:
    void applyStateSprite() override;
    void updateStateTimers(float dt) override;

public:
    explicit BotomBlue(sf::Vector2f pos);

    void updateAI(float dt) override;
};