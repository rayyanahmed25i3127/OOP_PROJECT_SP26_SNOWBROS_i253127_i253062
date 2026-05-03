#include "Game.hpp"
#include "states/MenuState.hpp"
#include "states/LoginState.hpp"

Game::Game()
    : m_window(sf::VideoMode({800, 600}), "Snow Bros")
{
    m_window.setFramerateLimit(60);

    // FIX: setWindow MUST be called before pushState.
    // StateManager stores this pointer and passes it to every state via
    // state->setWindow(m_window) inside applyPendingActions().
    // Without this call, m_window is nullptr in every state, and any
    // state that calls m_window->getSize() or m_window->mapPixelToCoords()
    // (LoginState, SignupState, ShopState) crashes immediately with a
    // null-pointer dereference — causing the white-flash-then-close symptom.
    m_stateManager.setWindow(&m_window);

    m_stateManager.pushState(new LoginState());
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