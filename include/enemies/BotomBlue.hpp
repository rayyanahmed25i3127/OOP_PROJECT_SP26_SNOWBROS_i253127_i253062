#pragma once
#include "enemies/Enemy.hpp"

/**
 * @brief Blue variant of Botom — 3 hits to encase, 125% speed.
 *
 * Hit progression:
 *   1 hit  → PartialEncase  (botom_blue_encased25.png overlay)
 *   2 hits → PartialEncase  (botom_blue_encased50.png overlay)
 *   3 hits → Snowballed     (snow_encase_100.png overlay — ready to roll)
 *
 * Escape sequence (same timing as red Botom):
 *   Snowballed → Escaping75 → Escaping50 → Escaping25 → Alive
 *   Uses: botom_blue_unleashed75/50/25.png
 *
 * Animations: botom_blue_walk_frame1/2/3.png, botom_blue_jump.png, botom_blue_fall.png
 *
 * Inheritance: Entity → Enemy → Botom → BotomBlue  (depth 4)
 */
class BotomBlue : public Enemy {
private:
    // Extra partial encasement texture (75% stage)
    sf::Texture m_encased25Texture;   // botom_blue_encased25.png  (hit 1)
    sf::Texture m_encased50Texture;   // botom_blue_encased50.png  (hit 2)
    bool m_encased25Loaded;
    bool m_encased50Loaded;

    // m_hitsTaken in Enemy base tracks hit count (1→PartialEncase, 2→PartialEncase, 3→Snowballed)

    // Direction / jump AI timers (same logic as Botom)
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