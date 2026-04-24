#include "enemies/Botom.hpp"
#include <cstdlib>
#include <iostream>

namespace {
    const float BOTOM_SPRITE_W = 44.f;
    const float BOTOM_SPRITE_H = 42.f;

    const float BOTOM_HIT_W = 28.f;
    const float BOTOM_HIT_H = 34.f;

    const float BOTOM_SPEED = 80.f;

    const int   BOTOM_HITS_TO_ENCASE = 2;

    float randomBetween(float lo, float hi) {
        float t = static_cast<float>(std::rand()) /
                  static_cast<float>(RAND_MAX);
        return lo + t * (hi - lo);
    }
}

Botom::Botom(sf::Vector2f pos)
    : Enemy(pos,
            BOTOM_SPRITE_W, BOTOM_SPRITE_H,
            BOTOM_HIT_W,    BOTOM_HIT_H,
            BOTOM_SPEED,
            BOTOM_HITS_TO_ENCASE)
    , m_directionTimer(0.f)
    , m_minDirectionInterval(1.0f)
    , m_maxDirectionInterval(3.0f)
    , m_jumpTimer(0.f)
    , m_minJumpInterval(3.0f)
    , m_maxJumpInterval(6.0f)
    , m_lastWalkVelocityX(0.f)
{
    if (!m_texture.loadFromFile("assets/sprites/botom_idle.png")) {
        std::cerr << "[Botom] Failed to load botom_idle.png\n";
    } else {
        m_sprite.setTexture(m_texture, true);
    }

    m_facingRight = (std::rand() % 2 == 0);

    rollDirectionTimer();
    rollJumpTimer();

    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = BOTOM_SPRITE_W / static_cast<float>(texSize.x);
        float scaleY = BOTOM_SPRITE_H / static_cast<float>(texSize.y);
        m_sprite.setScale({ scaleX, scaleY });
    }
    m_sprite.setPosition(pos);
}

void Botom::rollDirectionTimer() {
    m_directionTimer = randomBetween(m_minDirectionInterval,
                                     m_maxDirectionInterval);
}

void Botom::rollJumpTimer() {
    m_jumpTimer = randomBetween(m_minJumpInterval, m_maxJumpInterval);
}

void Botom::updateAI(float dt) {
    if (m_state != State::Alive) {
        velocity.x = 0.f;
        return;
    }

    // --- Timed random direction flip ---
    m_directionTimer -= dt;
    if (m_directionTimer <= 0.f) {
        m_facingRight = !m_facingRight;
        rollDirectionTimer();
    }

    // --- Wall-bounce detection ---
    if (m_lastWalkVelocityX != 0.f && velocity.x == 0.f) {
        m_facingRight = !m_facingRight;
        rollDirectionTimer();
    }

    // --- Timed random jump ---
    // Only grounded Botoms "decide" to jump. Airborne ones continue
    // their trajectory. Enemy::jump() itself also guards on onGround
    // but we check here to avoid resetting the timer while in the air.
    m_jumpTimer -= dt;
    if (m_jumpTimer <= 0.f) {
        if (m_onGround) {
            jump();
            rollJumpTimer();
        } else {
            // Keep trying every frame until we land, then commit the jump
            // on the first grounded frame. Short hold so we don't perma-jump.
            m_jumpTimer = 0.f;
        }
    }

    // --- Apply walk velocity ---
    velocity.x = m_facingRight ? m_speed : -m_speed;

    m_lastWalkVelocityX = velocity.x;
}