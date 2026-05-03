#pragma once
#include "enemies/Enemy.hpp"

// botom, aik enemy hai jo zameen par chalta hai aur thori thori der baad 
// khud hi apni direction badal leta hai ya jump marta hai taake movement "natural" lage. 
// Agar yeh deewar se takraye toh foran mud jata hai aur 2 snowball hits par snoball me 
// encase ho jata hai.....yeh base class hai, jis se aage urrne wale enemies 
// (FlyngFoogaFoog wagera) banaye gaye hain.
class Botom : public Enemy {
private:
// dir flip counter
    float m_directionTimer;
    float m_minDirectionInterval;
    float m_maxDirectionInterval;

    // jump counter
    float m_jumpTimer;
    float m_minJumpInterval;
    float m_maxJumpInterval;

    // hit wall and flip
    float m_lastWalkVelocityX;

    void rollDirectionTimer();
    void rollJumpTimer();

public:
    explicit Botom(sf::Vector2f pos);

    void updateAI(float dt) override;
};