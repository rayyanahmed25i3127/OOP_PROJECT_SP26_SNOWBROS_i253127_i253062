#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

/**
 * @brief The home/main menu screen — first thing the player sees.
 *
 * Shows the game title and three buttons: Start Game, Leaderboard, Exit.
 *
 * Supports both mouse (click) and keyboard (Up/Down + Enter) navigation.
 * Whichever input method the user uses, the same action is triggered.
 *
 * Satisfies part of spec section 12.1 "Main Menu" and task #12 on the
 * work division sheet.
 */
class MenuState : public GameState {
private:
    // A single menu button. Using composition — MenuState HAS-A list of Buttons.
    // Each button owns its own text and shape, plus a callback for what to do
    // when clicked. The callback approach means we don't need a switch statement
    // or an enum to dispatch actions — clean and extensible.
    struct Button {
        sf::Text text;                    // label ("Start Game", etc.)
        sf::RectangleShape background;    // invisible-ish hit area behind text
        std::function<void()> onActivate; // what happens on click/enter
    };

    sf::Font m_font;
    sf::Text m_title;                     // "SNOW BROS" big at the top
    std::vector<Button> m_buttons;
    int m_selectedIndex = 0;              // which button is highlighted (for keyboard)

    // Colors — kept as constants so tweaking the theme is one-line
    const sf::Color m_colorNormal   = sf::Color(200, 200, 200);
    const sf::Color m_colorSelected = sf::Color(255, 220, 80);  // yellow highlight
    const sf::Color m_colorTitle    = sf::Color(120, 200, 255); // icy blue

    // Helper: build one button and add it to m_buttons.
    // Taking the callback as the last arg lets us use nice lambdas at the call site.
    void addButton(const std::string& label, float yPos, std::function<void()> onActivate);

    // Helper: update button visuals based on current selection index.
    void refreshButtonVisuals();

    // Helper: check if a given screen point is inside a button (for mouse).
    // Returns the button index, or -1 if no button is under the point.
    int buttonAtPoint(sf::Vector2f point) const;

public:
    MenuState();

    // Overrides from GameState
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
};