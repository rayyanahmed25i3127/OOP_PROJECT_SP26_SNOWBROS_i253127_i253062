#include "effects/SnowEffect.hpp"
#include <cstdlib>   
#include <ctime>     
#include <cmath>     

SnowEffect::SnowEffect(float windowWidth, float windowHeight)
    : m_windowWidth(windowWidth), m_windowHeight(windowHeight)
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }

    for (int i = 0; i < MAX_FLAKES; ++i) {
        m_flakes[i].shape.setPointCount(6);
        m_flakes[i].shape.setFillColor(sf::Color(255, 255, 255, 200)); 

        respawnFlake(i, true);
    }
}

void SnowEffect::respawnFlake(int index, bool anywhereOnScreen) {
    Snowflake& f = m_flakes[index];

    float radius = randFloat(1.5f, 4.5f);
    f.shape.setRadius(radius);

    // Random horizontal position across the whole screen width
    float x = randFloat(0.f, m_windowWidth);

    float y = anywhereOnScreen
        ? randFloat(0.f, m_windowHeight)
        : randFloat(-50.f, -5.f);

    f.shape.setPosition({x, y});

    f.velocityY = randFloat(20.f, 50.f) * (radius / 3.f);

    // Gentle horizontal breeze, mostly neutral
    f.velocityX = randFloat(-8.f, 8.f);

    f.swayTimer = 0.f;
    f.swayPhase = randFloat(0.f, 6.28318f);  // 0 to 2*PI
}

float SnowEffect::randFloat(float min, float max) {
    // rand() returns int in [0, RAND_MAX]. Normalize to [0, 1], scale to range.
    float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return min + t * (max - min);
}

void SnowEffect::update(float dt) {
    for (int i = 0; i < MAX_FLAKES; ++i) {
        Snowflake& f = m_flakes[i];

        f.swayTimer += dt;

        float swayOffset = std::sin(f.swayPhase + f.swayTimer * 0.8f) * 15.f;

        auto pos = f.shape.getPosition();
        pos.x += (f.velocityX + swayOffset) * dt;
        pos.y += f.velocityY * dt;

        f.shape.setPosition(pos);


        if (pos.y > m_windowHeight + 10.f) {
            respawnFlake(i, false);
        }
        else if (pos.x < -10.f) {
            pos.x = m_windowWidth + 5.f;
            f.shape.setPosition(pos);
        }
        else if (pos.x > m_windowWidth + 10.f) {
            pos.x = -5.f;
            f.shape.setPosition(pos);
        }
    }
}

void SnowEffect::draw(sf::RenderWindow& window) const {
    for (int i = 0; i < MAX_FLAKES; ++i) {
        window.draw(m_flakes[i].shape);
    }
}