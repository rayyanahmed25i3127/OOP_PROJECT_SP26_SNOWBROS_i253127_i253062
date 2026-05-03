#include "states/SplashState.hpp"
#include "states/StateManager.hpp"
#include "states/LoginState.hpp"
#include "audio/AudioManager.hpp"
#include <iostream>

SplashState::SplashState() : m_bg(nullptr), m_fontLoaded(false), m_promptText(nullptr), m_blinkTimer(0.f), m_blinkVisible(true) {}
SplashState::~SplashState() { delete m_bg; delete m_promptText; }

void SplashState::onEnter() {
    AudioManager::get().playMenuMusic();
    if (m_bgTexture.loadFromFile("assets/sprites/splash_bg.png")) {
        m_bg = new sf::Sprite(m_bgTexture);
        sf::Vector2u sz = m_bgTexture.getSize();
        if (sz.x > 0 && sz.y > 0) m_bg->setScale({WINDOW_W/(float)sz.x, WINDOW_H/(float)sz.y});
    }
    if (m_font.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) m_fontLoaded = true;
    if (m_fontLoaded) {
        m_promptText = new sf::Text(m_font, "Press any key to Continue...", 10);
        m_promptText->setFillColor(sf::Color::Black);
        sf::FloatRect tb = m_promptText->getLocalBounds();
        m_promptText->setPosition({(WINDOW_W-tb.size.x)/2.f-tb.position.x, WINDOW_H-tb.size.y-tb.position.y-18.f});
    }
}
void SplashState::onExit() {}

void SplashState::handleEvent(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>())
        m_manager->replaceState(new LoginState());
}
void SplashState::update(float dt) {
    m_blinkTimer += dt;
    if (m_blinkTimer >= BLINK_INTERVAL) { m_blinkTimer -= BLINK_INTERVAL; m_blinkVisible = !m_blinkVisible; }
}
void SplashState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    if (m_bg) window.draw(*m_bg);
    if (m_promptText && m_blinkVisible) window.draw(*m_promptText);
}