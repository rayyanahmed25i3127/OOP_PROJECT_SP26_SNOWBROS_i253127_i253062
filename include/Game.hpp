#pragma once

#include <SFML/Graphics.hpp>
#include "states/StateManager.hpp"


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