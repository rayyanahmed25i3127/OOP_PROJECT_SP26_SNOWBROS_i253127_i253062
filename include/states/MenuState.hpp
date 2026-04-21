#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief The home/main menu screen — first thing the player sees.
 *
 * Shows the game title and three buttons: Start Game, Leaderboard, Exit.
 * Supports both mouse (click/hover) and keyboard (Up/Down + Enter) navigation.
 *
 * Uses fixed-size C-style arrays instead of STL containers, and an enum +
 * switch for button actions instead of std::function callbacks.
 */
class MenuState : public GameState {
public:
    // Maximum number of menu buttons. 3 for now (Start/Leaderboard/Exit)
    // but keeping a constant makes it easy to add Settings/Credits later.
    static const int MAX_BUTTONS = 8;

private:
    // Each button has an ID used to dispatch its action in a switch statement.
    // Enum-based dispatch is the classic OOP alternative to callback pointers.
    enum class ButtonAction {
        StartGame,
        Leaderboard,
        Exit
    };

    // A single menu button — composition: MenuState HAS-A array of Buttons.
    struct Button {
        sf::Text text;                  // label ("Start Game", etc.)
        sf::RectangleShape background;  // rectangle behind the text
        ButtonAction action;            // what to do when activated

        // sf::Text requires a font reference on construction (SFML 3 quirk),
        // so we need a constructor that takes the font.
        Button(const sf::Font& font);
    };

    sf::Font m_font;
    sf::Text m_title;

    Button* m_buttons[MAX_BUTTONS];  // array of pointers (Button has no default ctor in SFML 3)
    int m_buttonCount;

    int m_selectedIndex;

    // Theme colors
    const sf::Color m_colorNormal   = sf::Color(200, 200, 200);
    const sf::Color m_colorSelected = sf::Color(255, 220, 80);
    const sf::Color m_colorTitle    = sf::Color(120, 200, 255);

    // Helpers
    void addButton(const std::string& label, float yPos, ButtonAction action);
    void refreshButtonVisuals();
    int buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);  // runs the action at index

public:
    MenuState();
    ~MenuState();

    // Disable copy (we own heap Button objects)
    MenuState(const MenuState&) = delete;
    MenuState& operator=(const MenuState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
};