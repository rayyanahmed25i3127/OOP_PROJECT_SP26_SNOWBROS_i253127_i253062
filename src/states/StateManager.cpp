#include "states/StateManager.hpp"
#include <iostream>

StateManager::StateManager()
    : m_stateCount(0), m_pendingCount(0)
{
    // Initialize all slots to nullptr for safety
    for (int i = 0; i < MAX_STATES; ++i) {
        m_states[i] = nullptr;
    }
    for (int i = 0; i < MAX_PENDING; ++i) {
        m_pendingActions[i].state = nullptr;
    }
}

StateManager::~StateManager() {
    // We own every state in the stack — delete them all.
    // Also delete any pending states that never got applied
    // (edge case: if the window closes before applyPendingActions runs).
    while (m_stateCount > 0) {
        --m_stateCount;
        delete m_states[m_stateCount];
    }
    for (int i = 0; i < m_pendingCount; ++i) {
        delete m_pendingActions[i].state;  // delete nullptr is safe
    }
}

void StateManager::pushState(GameState* state) {
    if (m_pendingCount >= MAX_PENDING) {
        std::cerr << "[StateManager] Pending action queue full, dropping push\n";
        delete state;  // we took ownership — must clean up
        return;
    }
    m_pendingActions[m_pendingCount].type = ActionType::Push;
    m_pendingActions[m_pendingCount].state = state;
    ++m_pendingCount;
}

void StateManager::popState() {
    if (m_pendingCount >= MAX_PENDING) {
        std::cerr << "[StateManager] Pending action queue full, dropping pop\n";
        return;
    }
    m_pendingActions[m_pendingCount].type = ActionType::Pop;
    m_pendingActions[m_pendingCount].state = nullptr;
    ++m_pendingCount;
}

void StateManager::replaceState(GameState* state) {
    if (m_pendingCount >= MAX_PENDING) {
        std::cerr << "[StateManager] Pending action queue full, dropping replace\n";
        delete state;
        return;
    }
    m_pendingActions[m_pendingCount].type = ActionType::Replace;
    m_pendingActions[m_pendingCount].state = state;
    ++m_pendingCount;
}

void StateManager::applyPendingActions() {
    // Process in a loop because onEnter()/onExit() might queue more actions.
    while (m_pendingCount > 0) {
        // Snapshot the current queue so new actions queued during processing
        // are applied on the NEXT iteration, not this one.
        PendingAction snapshot[MAX_PENDING];
        int snapshotCount = m_pendingCount;
        for (int i = 0; i < snapshotCount; ++i) {
            snapshot[i] = m_pendingActions[i];
        }
        m_pendingCount = 0;

        for (int i = 0; i < snapshotCount; ++i) {
            PendingAction& action = snapshot[i];

            if (action.type == ActionType::Push) {
                if (m_stateCount >= MAX_STATES) {
                    std::cerr << "[StateManager] State stack full, dropping push\n";
                    delete action.state;
                    continue;
                }
                action.state->setManager(this);
               action.state->setWindow(m_window); 
                action.state->onEnter();
                m_states[m_stateCount] = action.state;
                ++m_stateCount;
            }
            else if (action.type == ActionType::Pop) {
                if (m_stateCount > 0) {
                    --m_stateCount;
                    m_states[m_stateCount]->onExit();
                    delete m_states[m_stateCount];
                    m_states[m_stateCount] = nullptr;
                }
            }
            else if (action.type == ActionType::Replace) {
                // Clear the entire stack first
                while (m_stateCount > 0) {
                    --m_stateCount;
                    m_states[m_stateCount]->onExit();
                    delete m_states[m_stateCount];
                    m_states[m_stateCount] = nullptr;
                }
                // Then push the new state
                action.state->setManager(this);
                action.state->onEnter();
                m_states[0] = action.state;
                m_stateCount = 1;
            }
        }
    }
}

void StateManager::handleEvent(const sf::Event& event) {
    if (m_stateCount > 0) {
        m_states[m_stateCount - 1]->handleEvent(event);
    }
}

void StateManager::update(float dt) {
    if (m_stateCount > 0) {
        m_states[m_stateCount - 1]->update(dt);
    }
}

void StateManager::draw(sf::RenderWindow& window) {
    if (m_stateCount == 0) return;

    // Walk backwards from top to find the deepest "opaque" state.
    // Then draw from there up to the top.
    int firstToDraw = m_stateCount - 1;
    while (firstToDraw > 0 && m_states[firstToDraw]->isTransparent()) {
        --firstToDraw;
    }
    for (int i = firstToDraw; i < m_stateCount; ++i) {
        m_states[i]->draw(window);
    }
}
