#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

//Yeh overlay transparent hai taake peeche ruki hui game nazar aa sake aur is mein 
//teen buttons hain: Main Menu, Continue, aur Exit Game. "Continue" button dabane
//par aapke gems reduce hain (jin ki qeemat har bar barhti jati hai) aur aap dobara 
//zinda ho sakte hain. Continue button ke sath hi gems ka icon aur un ki qeemat (cost) 
//bhi dikhayi deti hai taake player ko pata chale ke kitne gems kam honge.
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

    // kitne paise label for new life
    sf::Text             m_costText;

    //gem icon 
    sf::Texture          m_gemTexture;
    sf::Sprite           m_gemSprite;
    bool                 m_gemLoaded;

    // cont cost cached everytime
    int                  m_continueCost;

    void setHovered(int index);
    int  buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);
    void refreshCostLabel();          // updates m_costText string nd pos

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