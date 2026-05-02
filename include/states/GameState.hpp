#pragma once

#include <SFML/Graphics.hpp>

// Forward declaration
class StateManager;

class GameState {
protected:
    // Ã°Å¸â€Â¥ ONLY ONCE
    StateManager* m_manager = nullptr;
    sf::RenderWindow* m_window = nullptr;

public:
    virtual bool isTransparent() const { return false; }

    virtual ~GameState() = default;

    // Ã°Å¸â€Â§ Setters
    void setManager(StateManager* manager) { m_manager = manager; }
    void setWindow(sf::RenderWindow* window) { m_window = window; }

    // Core loop functions
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    // Lifecycle
    virtual void onEnter() {}
    virtual void onExit() {}
};