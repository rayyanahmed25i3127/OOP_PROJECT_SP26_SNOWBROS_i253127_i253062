#include "states/StateManager.hpp"

void StateManager::pushState(StatePtr state) {
    // Queue the push. We don't modify the stack immediately because
    // this might be called FROM a state's update() — modifying the stack
    // mid-iteration would invalidate references.
    m_pendingActions.push_back({ ActionType::Push, std::move(state) });
}

void StateManager::popState() {
    // Pop has no payload state.
    m_pendingActions.push_back({ ActionType::Pop, nullptr });
}

void StateManager::replaceState(StatePtr state) {
    m_pendingActions.push_back({ ActionType::Replace, std::move(state) });
}

void StateManager::applyPendingActions() {
    // Execute all queued actions, then clear the queue.
    // We move the queue out first so that new actions queued during
    // onEnter()/onExit() get processed on the next frame, not this loop.
    while (!m_pendingActions.empty()) {
        auto actions = std::move(m_pendingActions);
        m_pendingActions.clear();

        for (auto& action : actions) {
            switch (action.type) {
                case ActionType::Push:
                    action.state->setManager(this);
                    action.state->onEnter();
                    m_states.push_back(std::move(action.state));
                    break;

                case ActionType::Pop:
                    if (!m_states.empty()) {
                        m_states.back()->onExit();
                        m_states.pop_back();
                    }
                    break;

                case ActionType::Replace:
                    // Clear everything first
                    while (!m_states.empty()) {
                        m_states.back()->onExit();
                        m_states.pop_back();
                    }
                    action.state->setManager(this);
                    action.state->onEnter();
                    m_states.push_back(std::move(action.state));
                    break;
            }
        }
    }
}

void StateManager::handleEvent(const sf::Event& event) {
    if (!m_states.empty()) {
        m_states.back()->handleEvent(event);
    }
}

void StateManager::update(float dt) {
    if (!m_states.empty()) {
        m_states.back()->update(dt);
    }
}

void StateManager::draw(sf::RenderWindow& window) {
    if (!m_states.empty()) {
        m_states.back()->draw(window);
    }
}