#include "Game.hpp"
#include "states/MenuState.hpp"

Game::Game()
    : m_window(sf::VideoMode({800, 600}), "Snow Bros")
{
    m_window.setFramerateLimit(60);

    // Push MenuState as the initial state.
    // `new MenuState()` allocates on the heap; StateManager takes ownership
    // and will delete it when the state is popped or when StateManager itself
    // is destroyed.
     m_stateManager.setWindow(&m_window);
    m_stateManager.pushState(new MenuState());
}

void Game::run() {
    while (m_window.isOpen()) {
        float dt = m_clock.restart().asSeconds();

        while (const std::optional event = m_window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                m_window.close();
            }
            m_stateManager.handleEvent(*event);
        }

        m_stateManager.applyPendingActions();

        if (m_stateManager.isEmpty()) {
            m_window.close();
            break;
        }

        m_stateManager.update(dt);

        m_window.clear(sf::Color(15, 20, 35));
        m_stateManager.draw(m_window);
        m_window.display();
    }
}