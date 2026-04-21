#pragma once

#include "states/GameState.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief The main gameplay state.
 *
 * Wraps Anas's Player + Platform objects inside the state machine architecture.
 * Owns them as raw pointers (no STL, no smart pointers per course rules).
 *
 * Responsibilities:
 *   - Load gameplay background image
 *   - Create & update Player with Anas's movement code
 *   - Create & draw Platforms
 *   - Push PauseState when Escape is pressed
 *   - Switch to game music when entered
 *
 * Hitbox debug toggle (F1) is handled here per spec section 7.2.
 */
class PlayState : public GameState {
public:
    // Hard cap on number of platforms per level. Matches the fixed-array rule
    // from your course constraints. 16 is plenty for a single-screen Snow Bros level.
    static const int MAX_PLATFORMS = 16;

private:
    // Background
    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;
    bool m_backgroundLoaded;

    // Player — heap-allocated, owned by this state.
    Player* m_player;

    // Platforms — fixed array of owned pointers.
    Platform* m_platforms[MAX_PLATFORMS];
    int m_platformCount;

    // Debug toggle for hit-boxes (spec 7.2 / section 15.3: "Hit-box toggle must work without restarting")
    bool m_showHitboxes;

    // Helpers
    void buildLevel();  // creates the platforms for this level

public:
    PlayState();
    ~PlayState();

    // Disable copy (we own heap memory)
    PlayState(const PlayState&) = delete;
    PlayState& operator=(const PlayState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
    void onExit() override;
};