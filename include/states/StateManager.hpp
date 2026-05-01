#pragma once

#include "states/GameState.hpp"
#include "../UserManager.hpp"
#include <string>

class StateManager {
private:
    sf::RenderWindow* m_window = nullptr;

    std::string m_currentUser;
    UserManager m_userManager;

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

    // ── Shop purchase flags ────────────────────────────────────────────────
    // Each flag is set to true when bought in the shop.
    // PlayState reads them on onEnter(), applies the effect, then calls
    // resetBoughtPowerUps() so the same item can be bought again next visit.
    bool m_speedBought;
    bool m_snowballBought;
    bool m_distanceBought;
    bool m_balloonBought;
    bool m_extraLifeBought;

public:
    StateManager();
    ~StateManager();

    StateManager(const StateManager&)            = delete;
    StateManager& operator=(const StateManager&) = delete;

    // ── Current user ──────────────────────────────────────────────────────
    void        setCurrentUser(const std::string& name);
    std::string getCurrentUserName() const;

    UserManager& getUserManager();

    // ── State controls ────────────────────────────────────────────────────
    void pushState(GameState* state);
    void popState();
    void replaceState(GameState* state);

    void applyPendingActions();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void drawAll(sf::RenderWindow& window);

    bool isEmpty() const { return m_stateCount == 0; }

    // ── Shop: buy (called from ShopState) ────────────────────────────────
    void buySpeed()     { m_speedBought     = true; }
    void buySnowball()  { m_snowballBought  = true; }
    void buyDistance()  { m_distanceBought  = true; }
    void buyBalloon()   { m_balloonBought   = true; }
    void buyExtraLife() { m_extraLifeBought = true; }

    // ── Shop: query (called from ShopState to grey out already-bought) ───
    bool isSpeedBought()     const { return m_speedBought;     }
    bool isSnowballBought()  const { return m_snowballBought;  }
    bool isDistanceBought()  const { return m_distanceBought;  }
    bool isBalloonBought()   const { return m_balloonBought;   }
    bool isExtraLifeBought() const { return m_extraLifeBought; }

    // ── Called by PlayState after it has applied all purchased power-ups ──
    // Clears all flags so items become buyable again next shop visit.
    void resetBoughtPowerUps() {
        m_speedBought     = false;
        m_snowballBought  = false;
        m_distanceBought  = false;
        m_balloonBought   = false;
        m_extraLifeBought = false;
    }
};