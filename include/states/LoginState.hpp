#pragma once

#include "states/GameState.hpp"
#include "../UserManager.hpp"
#include <SFML/Graphics.hpp>

class LoginState : public GameState {
private:
    //font
    sf::Font m_font;

    //bg
    sf::Texture m_bgTexture;
    sf::Sprite* m_bgSprite;
    bool        m_bgLoaded;

    //state of input
    std::string m_username;
    std::string m_password;
    bool        m_typingUsername;
    bool        m_typingPassword;

   //txt objs
    sf::Text* m_titleText;
    sf::Text* m_usernameText;
    sf::Text* m_passwordText;
    sf::Text* m_infoText;
    sf::Text* m_loginText;
    sf::Text* m_signupText;
    sf::Text* m_tabHintText;

//hover effects
    bool m_loginHovered;
    bool m_signupHovered;

   //layout me contant cheezain buttons ki sizing wagera 
    static const float BTN_X;
    static const float BTN_W;
    static const float BTN_H;
    static const float LOGIN_Y;
    static const float SIGNUP_Y;

    //helpers
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
    LoginState();
    ~LoginState();

    void onEnter()                           override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
};