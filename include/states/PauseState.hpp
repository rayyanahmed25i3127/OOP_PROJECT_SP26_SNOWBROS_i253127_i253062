#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Pause menu — pushed on top of PlayState when player presses Escape.
 *
 * Classic "dimmed overlay" style: frozen gameplay shows through a
 * semi-transparent dark layer, with three buttons on top.
 *
 * Because this state sits on TOP of PlayState in the stack:
 *   - PlayState::update() is NOT called (game is frozen)
 *   - PlayState::draw() IS called first (via StateManager::drawAll in PauseState's own draw),
 *     so the frozen scene is visible behind the overlay
 *
 * Buttons:
 *   - Resume       -> pop self (returns to PlayState)
 *   - Main Menu    -> pop self AND pop PlayState (back to MenuState)
 *   - Exit Game    -> pop everything (Game::run closes window)
 */
class PauseState : public GameState {
public:
    static const int NUM_BUTTONS = 3;

private:
    enum class Action { Resume, MainMenu, Exit };

    struct Button {
        sf::Text text;
        sf::RectangleShape background;
        Action action;
        sf::Vector2f center;
        float baseWidth;
        float baseHeight;
        float currentScale;
        float targetScale;

        Button(const sf::Font& font);
        void configure(sf::Vector2f centerPos, float w, float h,
                       sf::Color fill, const std::string& label, Action act);
        void update(float dt);
        void draw(sf::RenderWindow& window) const;
    };

    sf::Font m_font;
    sf::Text m_title;

    // Dimmed overlay that covers the whole screen
    sf::RectangleShape m_overlay;

    Button* m_buttons[NUM_BUTTONS];
    int m_selectedIndex;

    // Helpers
    void setHovered(int index);
    int buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);

public:
    PauseState();
    ~PauseState();
    bool isTransparent() const override { return true; }
    PauseState(const PauseState&) = delete;
    PauseState& operator=(const PauseState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
};