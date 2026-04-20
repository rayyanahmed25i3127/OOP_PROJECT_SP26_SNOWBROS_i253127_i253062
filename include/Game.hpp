#pragma once

#include <SFML/Graphics.hpp>
#include "states/StateManager.hpp"

/**
 * @brief Top-level game class. Owns the window and the state manager.
 */
class Game {
private:
    sf::RenderWindow m_window;
    StateManager m_stateManager;
    sf::Clock m_clock;

public:
    Game();
    void run();

    StateManager& getStateManager() { return m_stateManager; }
};