#include "states/StateManager.hpp"
#include <iostream>

StateManager::StateManager()
    : m_stateCount(0)
    , m_pendingCount(0)
    , m_speedBought(false)
    , m_snowballBought(false)
    , m_distanceBought(false)
    , m_balloonBought(false)
    , m_extraLifeBought(false)
{
    for (int i = 0; i < MAX_STATES;  ++i) m_states[i]               = nullptr;
    for (int i = 0; i < MAX_PENDING; ++i) m_pendingActions[i].state = nullptr;
}

StateManager::~StateManager()
{
    while (m_stateCount > 0) {
        --m_stateCount;
        delete m_states[m_stateCount];
    }
    for (int i = 0; i < m_pendingCount; ++i)
        delete m_pendingActions[i].state;
}

void StateManager::pushState(GameState* state)
{
    if (m_pendingCount >= MAX_PENDING) {
        std::cerr << "[StateManager] Queue full, dropping push\n";
        delete state;
        return;
    }
    m_pendingActions[m_pendingCount].type  = ActionType::Push;
    m_pendingActions[m_pendingCount].state = state;
    ++m_pendingCount;
}

void StateManager::popState()
{
    if (m_pendingCount >= MAX_PENDING) {
        std::cerr << "[StateManager] Queue full, dropping pop\n";
        return;
    }
    m_pendingActions[m_pendingCount].type  = ActionType::Pop;
    m_pendingActions[m_pendingCount].state = nullptr;
    ++m_pendingCount;
}

void StateManager::replaceState(GameState* state)
{
    if (m_pendingCount >= MAX_PENDING) {
        std::cerr << "[StateManager] Queue full, dropping replace\n";
        delete state;
        return;
    }
    m_pendingActions[m_pendingCount].type  = ActionType::Replace;
    m_pendingActions[m_pendingCount].state = state;
    ++m_pendingCount;
}

void StateManager::applyPendingActions()
{
    while (m_pendingCount > 0)
    {
        PendingAction snapshot[MAX_PENDING];
        int snapshotCount = m_pendingCount;
        for (int i = 0; i < snapshotCount; ++i) snapshot[i] = m_pendingActions[i];
        m_pendingCount = 0;

        for (int i = 0; i < snapshotCount; ++i)
        {
            PendingAction& action = snapshot[i];

            if (action.type == ActionType::Push)
            {
                if (m_stateCount >= MAX_STATES) {
                    std::cerr << "[StateManager] Stack full, dropping push\n";
                    delete action.state;
                    continue;
                }
                action.state->setManager(this);
                action.state->setWindow(m_window);
                action.state->onEnter();
                m_states[m_stateCount++] = action.state;
            }
            else if (action.type == ActionType::Pop)
            {
                if (m_stateCount > 0) {
                    --m_stateCount;
                    m_states[m_stateCount]->onExit();
                    delete m_states[m_stateCount];
                    m_states[m_stateCount] = nullptr;
                }
            }
            else if (action.type == ActionType::Replace)
            {
                while (m_stateCount > 0) {
                    --m_stateCount;
                    m_states[m_stateCount]->onExit();
                    delete m_states[m_stateCount];
                    m_states[m_stateCount] = nullptr;
                }
                action.state->setManager(this);
                action.state->setWindow(m_window);
                action.state->onEnter();
                m_states[0]  = action.state;
                m_stateCount = 1;
            }
        }
    }
}

void StateManager::handleEvent(const sf::Event& event)
{
    if (m_stateCount > 0)
        m_states[m_stateCount - 1]->handleEvent(event);
}

void StateManager::update(float dt)
{
    if (m_stateCount > 0)
        m_states[m_stateCount - 1]->update(dt);
}

void StateManager::draw(sf::RenderWindow& window)
{
    if (m_stateCount == 0) return;
    int first = m_stateCount - 1;
    while (first > 0 && m_states[first]->isTransparent()) --first;
    for (int i = first; i < m_stateCount; ++i) m_states[i]->draw(window);
}

void StateManager::drawAll(sf::RenderWindow& window)
{
    for (int i = 0; i < m_stateCount; ++i) m_states[i]->draw(window);
}

void StateManager::setCurrentUser(const std::string& name) { m_currentUser = name; }
std::string StateManager::getCurrentUserName() const       { return m_currentUser; }
UserManager& StateManager::getUserManager()                { return m_userManager; }