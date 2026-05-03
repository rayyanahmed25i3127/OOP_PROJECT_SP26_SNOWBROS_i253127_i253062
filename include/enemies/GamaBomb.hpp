#pragma once
#include <SFML/Graphics.hpp>

/**
 * GamaBomb — projectile fired by Gamakichi toward a danger-zone target.
 * Falls under gravity, explodes on hitting target Y or player proximity.
 * Explosion lingers for EXPLODE_DURATION showing hit_effect sprite.
 */
class GamaBomb {
public:
    enum class Phase { Flying, Exploding, Dead };

    GamaBomb(sf::Vector2f start, sf::Vector2f target);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isDead()      const { return m_phase == Phase::Dead; }
    bool isExploding() const { return m_phase == Phase::Exploding; }
    sf::FloatRect getBlastRect() const;   // used for player hit check

private:
    sf::Texture m_bombTex;   bool m_bombLoaded;
    sf::Texture m_explTex;   bool m_explLoaded;
    sf::Sprite  m_bombSprite;
    sf::Sprite  m_explSprite;

    sf::Vector2f m_pos;
    sf::Vector2f m_target;
    sf::Vector2f m_velocity;
    Phase        m_phase;
    float        m_explTimer;

    static constexpr float BLAST_RADIUS    = 60.f;
    static constexpr float EXPLODE_DURATION = 0.6f;
    static constexpr float GRAVITY         = 500.f;
};