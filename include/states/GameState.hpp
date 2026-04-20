#pragma once

#include <SFML/Graphics.hpp>

// Forward declaration — avoids circular #include between GameState and StateManager.
class StateManager;

/**
 * @brief Abstract base class for all game states (screens).
 *
 * Every screen in Snow Bros — MenuState, PlayState, PauseState,
 * LeaderboardState, GameOverState — inherits from this class.
 *
 * Lifecycle each frame:
 *   1. handleEvent() — input events
 *   2. update(dt)    — game logic
 *   3. draw(window)  — rendering
 */
class GameState {
protected:
    // Non-owning pointer back to the manager, so states can request transitions.
    StateManager* m_manager = nullptr;

public:
    // Virtual destructor required for polymorphic deletion through base pointer.
    virtual ~GameState() = default;

    void setManager(StateManager* manager) { m_manager = manager; }

    // Pure virtual — derived classes MUST override these.
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    // Virtual but not pure — derived classes may override.
    virtual void onEnter() {}
    virtual void onExit() {}
};