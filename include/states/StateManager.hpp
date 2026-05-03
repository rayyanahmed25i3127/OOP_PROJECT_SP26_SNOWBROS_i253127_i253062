#pragma once

#include "states/GameState.hpp"
#include "../UserManager.hpp"
#include "../PlayerProgress.hpp"
#include <string>

class StateManager {
private:
    sf::RenderWindow* m_window = nullptr;

    std::string m_currentUser;
    UserManager m_userManager;
    PlayerProgress m_progress;

public:
    void setWindow(sf::RenderWindow* window) { m_window = window; }

    static const int MAX_STATES  = 8;
    static const int MAX_PENDING = 8;

private:
    enum class ActionType { Push, Pop, Replace };

    struct PendingAction {
        ActionType type;
        GameState* state;
    };

    GameState*    m_states[MAX_STATES];
    int           m_stateCount;

    PendingAction m_pendingActions[MAX_PENDING];
    int           m_pendingCount;
    bool m_balloonBought = false;
bool m_distanceBought = false;
bool m_snowballBought = false;
bool m_speedBought = false;
bool m_extraLifeBought = false;

public:
    StateManager();
    ~StateManager();
bool isBalloonBought() const { return m_balloonBought; }
bool isDistanceBought() const { return m_distanceBought; }
bool isSnowballBought() const { return m_snowballBought; }
bool isSpeedBought() const { return m_speedBought; }
bool isExtraLifeBought() const { return m_extraLifeBought; }
    StateManager(const StateManager&)            = delete;
    StateManager& operator=(const StateManager&) = delete;

    void        setCurrentUser(const std::string& name);
    std::string getCurrentUserName() const;

    UserManager& getUserManager();
    PlayerProgress& getProgress() { return m_progress; }

    void pushState(GameState* state);
    void popState();
    void replaceState(GameState* state);

    void applyPendingActions();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void drawAll(sf::RenderWindow& window);

    bool isEmpty() const { return m_stateCount == 0; }
    void setBalloonBought(bool val) { m_balloonBought = val; }
void setDistanceBought(bool val) { m_distanceBought = val; }
void setSnowballBought(bool val) { m_snowballBought = val; }
void setSpeedBought(bool val) { m_speedBought = val; }
void setExtraLifeBought(bool val) { m_extraLifeBought = val; }
};