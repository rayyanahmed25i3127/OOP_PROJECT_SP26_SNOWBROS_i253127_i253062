#include "enemies/MogeraChild.hpp"
#include "Platform.hpp"
#include <iostream>
#include <cstdio>

MogeraChild::MogeraChild(sf::Vector2f pos, sf::Vector2f initialVelocity)
    : Entity(pos)
    , m_sprite(m_ballTexture)        
    , m_ballLoaded(false)
    , m_walkLoaded(false)
    , m_phase(Phase::Ball)
    , m_walkFrame(0)
    , m_walkFrameTimer(0.f)
    , m_landedY(0.f)
    , m_spriteW(32.f)
    , m_spriteH(32.f)
{
    velocity = initialVelocity;

    if (std::FILE* f = std::fopen("assets/sprites/mogera_baby_ball.png", "rb")) {
        std::fclose(f);
        if (m_ballTexture.loadFromFile("assets/sprites/mogera_baby_ball.png")) {
            m_ballLoaded = true;
        } else {
            std::cerr << "[MogeraChild] decode fail: mogera_baby_ball.png\n";
        }
    } else {
        std::cerr << "[MogeraChild] missing: mogera_baby_ball.png\n";
    }

    m_walkLoaded = true;
    for (int i = 0; i < 2; ++i) {
        std::string path = "assets/sprites/mogera_baby_walk_frame"
                         + std::to_string(i + 1) + ".png";
        if (std::FILE* f = std::fopen(path.c_str(), "rb")) {
            std::fclose(f);
            if (!m_walkTextures[i].loadFromFile(path)) {
                std::cerr << "[MogeraChild] decode fail: " << path << "\n";
                m_walkLoaded = false;
            }
        } else {
            std::cerr << "[MogeraChild] missing: " << path << "\n";
            m_walkLoaded = false;
        }
    }

    if (m_ballLoaded) {
        m_sprite.setTexture(m_ballTexture, true);
    }

    std::cout << "[MogeraChild] ball=" << m_ballLoaded
              << " walk=" << m_walkLoaded << "\n";
    hitBox.size     = { m_spriteW, m_spriteH };
    hitBox.position = pos;

    syncSpriteToPosition();
}

void MogeraChild::syncSpriteToPosition() {
    sf::Texture* tex = nullptr;

    if (m_phase == Phase::Ball) {
        if (m_ballLoaded) tex = &m_ballTexture;
    } else {
        // Walking phase
        if (m_walkLoaded) {
            tex = &m_walkTextures[m_walkFrame];   // m_walkFrame is always 0 or 1
        } else if (m_ballLoaded) {
            tex = &m_ballTexture;                 // fallback if PNGs missing
        }
    }

    if (tex) {
        m_sprite.setTexture(*tex, true);

        auto ts = tex->getSize();
        if (ts.x > 0 && ts.y > 0) {
            m_sprite.setScale({
                m_spriteW / static_cast<float>(ts.x),
                m_spriteH / static_cast<float>(ts.y)
            });
        }
    }

    m_sprite.setPosition(position);
    hitBox.position = position;
}

extern Platform** g_platforms;
extern int        g_platformCount;

void MogeraChild::update(float dt) {
    if (!alive) return;

    if (m_phase == Phase::Ball) {
        velocity.y += GRAVITY * dt;
        position   += velocity * dt;
        syncSpriteToPosition();

        if (position.x + m_spriteW < 0.f) { alive = false; return; }
        if (position.y > 620.f)            { alive = false; return; }

    } else if (m_phase == Phase::Walking) {
        m_walkFrameTimer += dt;
        if (m_walkFrameTimer >= WALK_FRAME_TIME) {
            m_walkFrameTimer -= WALK_FRAME_TIME;
            m_walkFrame = (m_walkFrame + 1) % 2;
            if (m_walkLoaded) {
                m_sprite.setTexture(m_walkTextures[m_walkFrame], true);
                auto ts = m_walkTextures[m_walkFrame].getSize();
                if (ts.x > 0 && ts.y > 0) {
                    m_sprite.setScale({
                        m_spriteW / static_cast<float>(ts.x),
                        m_spriteH / static_cast<float>(ts.y)
                    });
                }
            }
        }

        velocity.x  = -WALK_SPEED;
        velocity.y += GRAVITY * dt;
        position   += velocity * dt;

        float bottom     = position.y + m_spriteH;
        float prevBottom = (position.y - velocity.y * dt) + m_spriteH; // approx prev bottom

        for (int i = 0; i < g_platformCount; ++i) {
            if (!g_platforms[i]) continue;
            for (int b = 0; b < g_platforms[i]->getHitboxCount(); ++b) {
                sf::FloatRect plat = g_platforms[i]->getBounds(b);

                // Horizontal overlap check
                float babyLeft  = position.x;
                float babyRight = position.x + m_spriteW;
                if (babyRight <= plat.position.x || babyLeft >= plat.position.x + plat.size.x)
                    continue;

                float platTop = plat.position.y;

                // Landing: was above, now at or below platform top, falling
                bool wasAbove = (prevBottom <= platTop + 2.f);
                bool nowBelow = (bottom      >= platTop);
                bool falling  = (velocity.y  >= 0.f);

                if (wasAbove && nowBelow && falling) {
                    position.y  = platTop - m_spriteH;
                    velocity.y  = 0.f;
                    m_landedY   = position.y;   // update floor clamp to new platform
                    break;
                }
            }
        }

        // Fallback ground at y=570 (same as CollisionDetector)
        const float GROUND_BOTTOM = 570.f;
        if (position.y + m_spriteH >= GROUND_BOTTOM) {
            position.y = GROUND_BOTTOM - m_spriteH;
            velocity.y = 0.f;
            m_landedY  = position.y;
        }

        
        m_sprite.setPosition(position);
        hitBox.position = position;

        if (position.x + m_spriteW < 0.f) { alive = false; return; }
    }
}

void MogeraChild::draw(sf::RenderWindow& window) {
    if (!alive) return;

    if (m_ballLoaded || m_walkLoaded) {
        window.draw(m_sprite);
    } else {
        // Fallback: small orange square
        sf::RectangleShape fb({ m_spriteW, m_spriteH });
        fb.setFillColor(sf::Color(220, 100, 40));
        fb.setOutlineColor(sf::Color::White);
        fb.setOutlineThickness(1.f);
        fb.setPosition(position);
        window.draw(fb);
    }
}
void MogeraChild::setPosition(sf::Vector2f pos) {
    position = pos;
    syncSpriteToPosition();
}


void MogeraChild::setOnGround(bool v) {
    if (v && m_phase == Phase::Ball) {
        m_phase          = Phase::Walking;
        // Record the Y position at landing so the walking phase can floor-clamp.
        m_landedY        = position.y;
        velocity.y       = 0.f;
        velocity.x       = -WALK_SPEED;
        m_walkFrame      = 0;
        m_walkFrameTimer = 0.f;
        syncSpriteToPosition();   // swap to walk texture immediately
        std::cout << "[MogeraChild] Landed y=" << m_landedY
                  << " walkLoaded=" << m_walkLoaded
                  << " phase=Walking\n";
    }
}

void MogeraChild::takeHit() {
    alive = false;
    std::cout << "[MogeraChild] Hit by snowball — dead\n";
}