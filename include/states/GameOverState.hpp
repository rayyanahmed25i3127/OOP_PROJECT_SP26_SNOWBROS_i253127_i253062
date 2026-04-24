#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Game-over overlay shown when the player dies.
 *
 * Pushed on top of PlayState when Player-Enemy contact is detected.
 * Like PauseState, this state is transparent (isTransparent() == true),
 * so the frozen scene remains visible behind the dimmed overlay.
 *
 * Structurally identical to PauseState but with different title/buttons:
 *   - Main Menu  -> pops self + PlayState, back to MenuState
 *   - Exit Game  -> pops entire stack, Game::run closes window
 */
class GameOverState : public GameState {
public:
    static const int NUM_BUTTONS = 2;

private:
    enum class Action { MainMenu, Exit };

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
    sf::RectangleShape m_overlay;

    Button* m_buttons[NUM_BUTTONS];
    int m_selectedIndex;

    void setHovered(int index);
    int  buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);

public:
    GameOverState();
    ~GameOverState();

    bool isTransparent() const override { return true; }

    GameOverState(const GameOverState&) = delete;
    GameOverState& operator=(const GameOverState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
};