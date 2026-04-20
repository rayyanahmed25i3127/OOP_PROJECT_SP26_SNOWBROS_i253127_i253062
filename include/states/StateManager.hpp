#pragma once

#include "states/GameState.hpp"
#include <memory>
#include <vector>

/**
 * @brief Manages a stack of game states.
 *
 * Stack allows pausing without destroying gameplay — PauseState sits on top
 * of PlayState, which stays alive underneath.
 *
 * Transitions are deferred to avoid modifying the stack during iteration.
 * We queue pending actions using a simple enum + optional state payload,
 * which works with move-only types like std::unique_ptr.
 */
class StateManager {
public:
    using StatePtr = std::unique_ptr<GameState>;

private:
    // Type of pending action
    enum class ActionType { Push, Pop, Replace };

    // A pending transition: what to do, and an optional new state for Push/Replace.
    struct PendingAction {
        ActionType type;
        StatePtr state;  // only used for Push and Replace; empty for Pop
    };

    std::vector<StatePtr> m_states;
    std::vector<PendingAction> m_pendingActions;

public:
    void pushState(StatePtr state);
    void popState();
    void replaceState(StatePtr state);

    void applyPendingActions();

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isEmpty() const { return m_states.empty(); }
};