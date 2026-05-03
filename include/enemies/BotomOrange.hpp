#pragma once
#include "enemies/Enemy.hpp"

// Identical behaviour to BotomBlue — 3 hits to encase, 125% speed.
// Only difference: all asset filenames use "orange" instead of "blue".
class BotomOrange : public Enemy {
private:
    sf::Texture m_encased25Texture;
    sf::Texture m_encased50Texture;
    bool m_encased25Loaded;
    bool m_encased50Loaded;

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
    explicit BotomOrange(sf::Vector2f pos);
    void updateAI(float dt) override;
};