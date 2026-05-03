#pragma once

#include "states/GameState.hpp"
#include "../UserManager.hpp"
#include <SFML/Graphics.hpp>

class SignupState : public GameState {
private:
    
    sf::Texture m_bgTexture;
    sf::Sprite* m_bgSprite;
    bool        m_bgLoaded;

    
    sf::Font m_font;

    
    std::string m_username;
    std::string m_password;
    bool        m_typingUsername;
    bool        m_typingPassword;


    
    sf::Text* m_titleText;
    sf::Text* m_usernameText;
    sf::Text* m_passwordText;
    sf::Text* m_infoText;
    sf::Text* m_registerText;
    sf::Text* m_backText;
    sf::Text* m_tabHintText;

    
    bool m_registerHovered;
    bool m_backHovered;

    
    static const float BTN_X;
    static const float BTN_W;
    static const float BTN_H;
    static const float REGISTER_Y;
    static const float BACK_Y;

    //hlpers identical approach to LoginState
    void drawCapsule(sf::RenderWindow& window,
                     float x, float y,
                     float width, float height,
                     sf::Color fillColor,
                     sf::Color outlineColor    = sf::Color::Transparent,
                     float     outlineThickness = 0.f);

    void drawInputField(sf::RenderWindow& window,
                        float x, float y,
                        float width, float height,
                        bool  active);

public:
    SignupState();
    ~SignupState();

    void onEnter()                           override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
};