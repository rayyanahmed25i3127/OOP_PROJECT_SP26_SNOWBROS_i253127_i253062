#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Game-over overlay shown when the player dies.
 *
 * Three buttons:
 *   Main Menu  -> pops self + PlayState, returns to MenuState
 *   Continue   -> deducts gems (escalating cost), signals PlayState to revive
 *   Exit Game  -> pops entire stack
 *
 * A gem icon + "-N" cost label is drawn to the left of the Continue button.
 * isTransparent() == true so the frozen scene is visible behind the overlay.
 */
class GameOverState : public GameState {
public:
    static const int NUM_BUTTONS = 3;

private:
    enum class Action { MainMenu, Continue, Exit };

    struct Button {
        sf::Text             text;
        sf::RectangleShape   background;
        Action               action;
        sf::Vector2f         center;
        float                baseWidth;
        float                baseHeight;
        float                currentScale;
        float                targetScale;

        Button(const sf::Font& font);
        void configure(sf::Vector2f centerPos, float w, float h,
                       sf::Color fill, const std::string& label, Action act);
        void update(float dt);
        void draw(sf::RenderWindow& window) const;
    };

    sf::Font             m_font;
    sf::Text             m_title;
    sf::RectangleShape   m_overlay;

    Button*              m_buttons[NUM_BUTTONS];
    int                  m_selectedIndex;

    // Cost label drawn to the left of the Continue button
    sf::Text             m_costText;

    // Small gem icon next to the cost label
    sf::Texture          m_gemTexture;
    sf::Sprite           m_gemSprite;
    bool                 m_gemLoaded;

    // Cached continue cost (recomputed on enter / after purchase)
    int                  m_continueCost;

    void setHovered(int index);
    int  buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);
    void refreshCostLabel();          // updates m_costText string & position

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