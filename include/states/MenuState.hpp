#pragma once

#include "states/GameState.hpp"
#include "effects/SnowEffect.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

/**
 * @brief The home/main menu screen.
 *
 * Features:
 *   - Background image with snowfall overlay
 *   - Three capsule-shaped "bubble" buttons with glossy shine
 *   - Per-button color theme (navy / green / red)
 *   - Smooth scale-up pop animation on hover
 *   - Bubble Bobble font for the button text
 *   - Mouse + keyboard navigation
 */
class MenuState : public GameState {
public:
    static const int MAX_BUTTONS = 8;

private:
    enum class ButtonAction {
        NewGame,
        Continue,
        Leaderboard,
        Logout,
        Exit
    };

    /**
     * @brief A capsule-shaped "bubble" button.
     *
     * SFML has no native rounded rectangle, so we fake the capsule with
     * a center rectangle + two end-cap circles. A semi-transparent white
     * ellipse on the upper half provides the glossy highlight.
     *
     * All of this is hidden behind the Button's own draw() method —
     * encapsulation in action.
     */
    struct Button {
        // Capsule parts (base color)
        sf::RectangleShape centerRect;
        sf::CircleShape    leftCap;
        sf::CircleShape    rightCap;

        // Glossy highlight on top (semi-transparent white ellipse)
        sf::CircleShape    shineLeft;
        sf::CircleShape    shineRight;
        sf::RectangleShape shineCenter;

        // Dark outline ring — drawn first, slightly larger than the capsule
        sf::RectangleShape outlineRect;
        sf::CircleShape    outlineLeft;
        sf::CircleShape    outlineRight;

        // Label
        sf::Text text;

        // Action + layout
        ButtonAction action;
        sf::Vector2f center;   // the button's center position (for scale anchor)
        float baseWidth;       // width at 1.0x scale
        float baseHeight;

        // Animation
        float currentScale;    // actually being rendered this frame
        float targetScale;     // 1.0 normal, 1.10 hovered

        // Theming
        sf::Color fillColor;

        Button(const sf::Font& font);

        // Build geometry from center position, size, color, label.
        void configure(sf::Vector2f centerPos,
                       float width, float height,
                       sf::Color baseFill,
                       const std::string& label);

        // Called every frame — smoothly eases currentScale toward targetScale,
        // then re-lays-out all the shapes based on the current scale.
        void update(float dt);

        // Draws all pieces in correct z-order.
        void draw(sf::RenderWindow& window) const;
    };

    // Fonts
    sf::Font m_bubbleFont;  // for buttons (BubbleBobble)

    // Background
    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;

    // Snow effect
    SnowEffect  m_snow;

    // Button click sound
    sf::SoundBuffer m_clickSoundBuffer;
    sf::Sound m_clickSound;
    bool m_clickSoundLoaded;

    // Buttons
    Button* m_buttons[MAX_BUTTONS];
    int m_buttonCount;
    int m_selectedIndex;

    // Helpers
    void addButton(const std::string& label, sf::Vector2f center,
                   sf::Color fillColor, ButtonAction action);
    void setHovered(int index);      // updates target scales
    int  buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);

public:
    MenuState();
    ~MenuState();

    MenuState(const MenuState&) = delete;
    MenuState& operator=(const MenuState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
};