#include "enemies/GamaBomb.hpp"
#include <cstdio>
#include <cmath>
#include <iostream>

GamaBomb::GamaBomb(sf::Vector2f start, sf::Vector2f target)
    : m_bombTex(), m_bombLoaded(false)
    , m_explTex(), m_explLoaded(false)
    , m_bombSprite(m_bombTex)
    , m_explSprite(m_explTex)
    , m_pos(start), m_target(target)
    , m_velocity(0.f, 0.f)
    , m_phase(Phase::Flying)
    , m_explTimer(0.f)
{
    auto tryLoad = [](sf::Texture& t, bool& f, const char* p) {
        if (std::FILE* fp = std::fopen(p,"rb")) { std::fclose(fp); f = t.loadFromFile(p); }
        else { std::cerr << "[GamaBomb] missing: " << p << "\n"; f = false; }
    };
    tryLoad(m_bombTex, m_bombLoaded, "assets/sprites/gamas_bombs_that_it_will_throw.png");
    tryLoad(m_explTex, m_explLoaded, "assets/sprites/hit_effect_where_the_bomb_will_hit.png");

    if (m_bombLoaded) { m_bombSprite.setTexture(m_bombTex, true);
        auto s = m_bombTex.getSize();
        if (s.x>0&&s.y>0) m_bombSprite.setScale({32.f/s.x, 32.f/s.y}); }
    if (m_explLoaded) { m_explSprite.setTexture(m_explTex, true);
        auto s = m_explTex.getSize();
        if (s.x>0&&s.y>0) m_explSprite.setScale({BLAST_RADIUS*2/s.x, BLAST_RADIUS*2/s.y}); }

    // Compute initial velocity for arc toward target
    sf::Vector2f d = target - start;
    float tFlight = 1.2f;
    m_velocity.x = d.x / tFlight;
    m_velocity.y = d.y / tFlight - 0.5f * GRAVITY * tFlight;
}

void GamaBomb::update(float dt) {
    if (m_phase == Phase::Dead) return;

    if (m_phase == Phase::Exploding) {
        m_explTimer += dt;
        if (m_explTimer >= EXPLODE_DURATION) m_phase = Phase::Dead;
        return;
    }

    // Flying
    m_velocity.y += GRAVITY * dt;
    m_pos.x += m_velocity.x * dt;
    m_pos.y += m_velocity.y * dt;
    m_bombSprite.setPosition(m_pos);

    // Explode when reaching target Y (or overshoots)
    if (m_pos.y >= m_target.y - 10.f) {
        m_pos = m_target;
        m_phase = Phase::Exploding;
        m_explTimer = 0.f;
        m_explSprite.setPosition({m_target.x - BLAST_RADIUS, m_target.y - BLAST_RADIUS});
    }
}

void GamaBomb::draw(sf::RenderWindow& window) {
    if (m_phase == Phase::Flying) {
        if (m_bombLoaded) window.draw(m_bombSprite);
        else {
            sf::CircleShape c(10.f); c.setFillColor(sf::Color(255,120,0));
            c.setPosition(m_pos); window.draw(c);
        }
    } else if (m_phase == Phase::Exploding) {
        if (m_explLoaded) window.draw(m_explSprite);
        else {
            sf::CircleShape c(BLAST_RADIUS); c.setFillColor(sf::Color(255,60,0,180));
            c.setPosition({m_target.x-BLAST_RADIUS, m_target.y-BLAST_RADIUS}); window.draw(c);
        }
    }
}

sf::FloatRect GamaBomb::getBlastRect() const {
    return { {m_target.x - BLAST_RADIUS, m_target.y - BLAST_RADIUS},
             {BLAST_RADIUS*2, BLAST_RADIUS*2} };
}