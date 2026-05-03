#pragma once

#include <SFML/Graphics.hpp>


class StateManager;

class GameState {
protected:
   
    StateManager* m_manager = nullptr;
    sf::RenderWindow* m_window = nullptr;

public:
    virtual bool isTransparent() const { return false; }

    virtual ~GameState() = default;
//setters
    void setManager(StateManager* manager) { m_manager = manager; }
    void setWindow(sf::RenderWindow* window) { m_window = window; }

   
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

//lifecycle
    virtual void onEnter() {}
    virtual void onExit() {}
};