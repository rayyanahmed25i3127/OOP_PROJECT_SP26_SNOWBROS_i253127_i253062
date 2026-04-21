#pragma once

#include "states/GameState.hpp"

/**
 * @brief Manages a fixed-capacity stack of game states.
 *
 * Uses a raw C-style array of GameState pointers (no STL containers).
 * Maximum stack depth is MAX_STATES (8) — more than enough for any
 * realistic UI flow (e.g., Menu -> Play -> Pause -> Shop is only 4 deep).
 *
 * Memory ownership: StateManager OWNS every state in the stack.
 * It is responsible for deleting them in the destructor and in pop/replace.
 * External code that calls pushState() hands over ownership via a raw pointer
 * obtained from `new`.
 *
 * Transitions are deferred to avoid modifying the stack during a state's
 * own update() or handleEvent().
 */
class StateManager {
public:
    // Maximum number of states we can stack at once.
    // Game flow: Menu -> Play -> Pause -> Shop = 4. 8 is a safe ceiling.
    static const int MAX_STATES = 8;

    // Maximum number of queued push/pop/replace actions per frame.
    static const int MAX_PENDING = 8;

private:
    // Pending action types
    enum class ActionType { Push, Pop, Replace };

    // A queued transition waiting to be applied
    struct PendingAction {
        ActionType type;
        GameState* state;  // raw pointer; only used for Push/Replace
    };

    // The state stack — fixed-size raw array of owning pointers
    GameState* m_states[MAX_STATES];
    int m_stateCount;

    // The queue of pending actions — also a fixed-size raw array
    PendingAction m_pendingActions[MAX_PENDING];
    int m_pendingCount;

public:
    StateManager();
    ~StateManager();

    // Disable copy — a StateManager owns heap memory and shouldn't be copied.
    // Rule of three: if you write a destructor, you should usually
    // disable or properly implement copy/move too.
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    // Takes ownership of `state` — caller must pass a pointer from `new`.
    void pushState(GameState* state);
    void popState();
    void replaceState(GameState* state);

    void applyPendingActions();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isEmpty() const { return m_stateCount == 0; }
};