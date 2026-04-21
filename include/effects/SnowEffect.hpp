#pragma once

#include <SFML/Graphics.hpp>

/**
 * @brief Reusable falling-snow particle system.
 *
 * Any state that wants ambient snow in the background simply owns one of
 * these as a member variable (composition). Call update(dt) each frame
 * and draw(window) once per frame before drawing foreground elements.
 *
 * States that should NOT have snow (like PlayState during gameplay)
 * simply don't own a SnowEffect.
 *
 * Uses a fixed-size raw C-style array of Snowflake — no STL.
 */
class SnowEffect {
public:
    // Max number of snowflakes on screen. 60 gives a gentle drift feel.
    // For a heavier blizzard, we'd bump this up.
    static const int MAX_FLAKES = 60;

private:
    // A single snowflake particle.
    // Composition: SnowEffect HAS-A array of Snowflake objects.
    struct Snowflake {
        sf::CircleShape shape;  // visual (6-sided polygon looks snowflake-ish)
        float velocityY;        // falling speed (pixels/second)
        float velocityX;        // horizontal drift (wind effect)
        float swayTimer;        // accumulated time, drives sine-wave sway
        float swayPhase;        // per-flake phase offset so they don't move in sync
    };

    Snowflake m_flakes[MAX_FLAKES];
    float m_windowWidth;
    float m_windowHeight;

    // Helper: place a snowflake at a fresh starting position.
    // Used on init and also to recycle flakes that fall off-screen.
    void respawnFlake(int index, bool anywhereOnScreen);

    // Returns a pseudo-random float in [min, max]. Uses rand() from <cstdlib>,
    // seeded once in the constructor. No <random> header (not in allowed list).
    float randFloat(float min, float max);

public:
    SnowEffect(float windowWidth, float windowHeight);

    // Per-frame update — advances each snowflake position.
    // dt = seconds since last frame (frame-rate independent).
    void update(float dt);

    // Draws all snowflakes. Call after drawing the background image
    // but before drawing the UI (buttons, text) so snow sits between them.
    void draw(sf::RenderWindow& window) const;
};