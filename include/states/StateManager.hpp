#pragma once

#include "states/GameState.hpp"
#include "../UserManager.hpp"
#include <string>

class StateManager {
private:
    sf::RenderWindow* m_window = nullptr;

    // 🔹 Logged-in user
    std::string m_currentUser;

    // 🔹 Shared UserManager (VERY IMPORTANT)
    UserManager m_userManager;

public:
    void setWindow(sf::RenderWindow* window) { m_window = window; }

    static const int MAX_STATES = 8;
    static const int MAX_PENDING = 8;

private:
    enum class ActionType { Push, Pop, Replace };

    struct PendingAction {
        ActionType type;
        GameState* state;
    };

    GameState* m_states[MAX_STATES];
    int m_stateCount;

    PendingAction m_pendingActions[MAX_PENDING];
    int m_pendingCount;

public:
    StateManager();
    ~StateManager();

    // ❌ disable copy
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    // 🔹 Current user
    void setCurrentUser(const std::string& name);
    std::string getCurrentUserName() const;

    // 🔹 Shared UserManager access
    UserManager& getUserManager();

    // ── State controls ──
    void pushState(GameState* state);
    void popState();
    void replaceState(GameState* state);

    void applyPendingActions();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void drawAll(sf::RenderWindow& window);

    bool isEmpty() const { return m_stateCount == 0; }
};