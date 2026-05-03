#pragma once

#include <SFML/Graphics.hpp>

// used for snowfall effect
class SnowEffect {
public:
    
    static const int MAX_FLAKES = 60; // no of snowflakes at the screen same time pe

private:
    // A single snowflake particle.
    // Composition yun hai k  snowEffect HAS-A array of Snowflake objects.
    struct Snowflake {
        sf::CircleShape shape;  // visual (6 sided polygon snowfall look k liay
        float velocityY;        // nichy any ki speed
        float velocityX;        // drift speeding 
        float swayTimer;        // drift time
        float swayPhase;        // for discrimination 
    };

    Snowflake m_flakes[MAX_FLAKES];
    float m_windowWidth;
    float m_windowHeight;

    void respawnFlake(int index, bool anywhereOnScreen);

    float randFloat(float min, float max); // min max em randomly koi number ayega rand func k zariye

public:
    SnowEffect(float windowWidth, float windowHeight);

    void update(float dt);

    void draw(sf::RenderWindow& window) const; // draws snow ball before buttons and all
};