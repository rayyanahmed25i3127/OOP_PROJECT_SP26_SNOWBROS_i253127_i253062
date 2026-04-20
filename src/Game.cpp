#include "Game.hpp"

Game::Game()
    : m_window(sf::VideoMode({800, 600}), "Snow Bros")
{
    m_window.setFramerateLimit(60);
    // No initial state pushed yet — window will close immediately.
    // We'll push MenuState here in the next step.
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

        m_window.clear(sf::Color::Black);
        m_stateManager.draw(m_window);
        m_window.display();
    }
}