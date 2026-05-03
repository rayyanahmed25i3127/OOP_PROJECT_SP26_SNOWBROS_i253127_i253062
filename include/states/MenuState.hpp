#pragma once

#include "states/GameState.hpp"
#include "effects/SnowEffect.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

// menu with snow effetc, bg image, 6 buttons capsule shapes diff colors diff wiriting hover effect buttons clickable sound, bubble bobble font style
class MenuState : public GameState {
public:
    static const int MAX_BUTTONS = 8;

private:
    enum class ButtonAction {
        NewGame,
        Multiplayer,
        Continue,
        Leaderboard,
        Logout,
        Exit
    };

    //menu buttons textured, with reflections nd bubble, used fake ellipse for capsule 
    struct Button {
        //capsule part 
        sf::RectangleShape centerRect;
        sf::CircleShape    leftCap;
        sf::CircleShape    rightCap;

        // reflection wlaa scene
        sf::CircleShape    shineLeft;
        sf::CircleShape    shineRight;
        sf::RectangleShape shineCenter;

        //outline black on buttons 
        sf::RectangleShape outlineRect;
        sf::CircleShape    outlineLeft;
        sf::CircleShape    outlineRight;

       //label, yani writing wagera
        sf::Text text;

        
        ButtonAction action;
        sf::Vector2f center;   
        float baseWidth;       
        float baseHeight;

        //hover effect for 1.10% 
        float currentScale;    // 1.0 normal, 1.10 hovered

        //coloring
        sf::Color fillColor;

        Button(const sf::Font& font);

        // actual loc of buttoms
        void configure(sf::Vector2f centerPos,
                       float width, float height,
                       sf::Color baseFill,
                       const std::string& label);

        
        void update(float dt); // updates with delta timestamp
//draws all buttons in correct order
        void draw(sf::RenderWindow& window) const;
    };

    //fonts
    sf::Font m_bubbleFont; 

    //bg
    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;

    //snowfall effect
    SnowEffect  m_snow;

    //click sound 
    sf::SoundBuffer m_clickSoundBuffer;
    sf::Sound m_clickSound;
    bool m_clickSoundLoaded;

    
    Button* m_buttons[MAX_BUTTONS];
    int m_buttonCount;
    int m_selectedIndex;

    //cannot cont wala message show karvane ka timer almost 2.8-3.0s fade out
    float m_cannotContinueTimer;

    //helpers
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