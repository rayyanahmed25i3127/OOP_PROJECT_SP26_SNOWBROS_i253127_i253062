#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

/**
 * @brief Pause menu â€” pushed on top of PlayState when player presses Escape.
 *
 * New design with background image and iOS-style glassy buttons.
 * - Background: pause_bg.png showing Snow Bros logo
 * - Title "PAUSED" below the logo
 * - 5 capsule-shaped glassy buttons with hover zoom effect
 *
 * Buttons:
 *   - Resume       -> pop self (returns to PlayState)
 *   - Main Menu    -> pop self AND pop PlayState (back to MenuState)
 *   - Shop         -> [TODO: implement shop state]
 *   - Logout       -> [TODO: implement logout]
 *   - Exit Game    -> pop everything (Game::run closes window)
 */
class PauseState : public GameState {
public:
    static const int NUM_BUTTONS = 5;

private:
    enum class Action { Resume, MainMenu, Shop, Logout, Exit };

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
                       const std::string& label, Action act);
        void update(float dt);
        void draw(sf::RenderWindow& window) const;
    };

    sf::Font m_font;
    sf::Text m_title;

    // Background image
    sf::Texture m_bgTexture;
    sf::Sprite m_bgSprite;
    bool m_bgLoaded;

    // Button click sound
    sf::SoundBuffer m_clickSoundBuffer;
    sf::Sound m_clickSound;
    bool m_clickSoundLoaded;

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