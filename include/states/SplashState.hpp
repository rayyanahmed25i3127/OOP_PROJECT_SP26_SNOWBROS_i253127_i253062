#pragma once
#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>

 
class SplashState : public GameState {
private:
    sf::Texture m_bgTexture;
    sf::Sprite* m_bg;
    sf::Font    m_font;
    bool        m_fontLoaded;
    sf::Text*   m_promptText;
    float       m_blinkTimer;
    bool        m_blinkVisible;
    static constexpr float BLINK_INTERVAL = 0.5f;
    static constexpr float WINDOW_W = 800.f;
    static constexpr float WINDOW_H = 600.f;
public:
    SplashState();
    ~SplashState() override;
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};