#pragma once
#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

// name entry for player 2, player 1 wahi hai jiska account hai 
class MultiplayerNameState : public GameState {
private:
    sf::Font   m_font;
    bool       m_fontLoaded = false;

    sf::Texture m_bgTexture;
    sf::Sprite* m_bg = nullptr;

    std::string m_p2Name;          // being typed
    bool        m_active = true;   // input field is focused

    static constexpr float W = 800.f;
    static constexpr float H = 600.f;

public:
    MultiplayerNameState() = default;
    ~MultiplayerNameState() override { delete m_bg; }

    void onEnter()  override;
    void onExit()   override {}
    void handleEvent(const sf::Event& event) override;
    void update(float)  override {}
    void draw(sf::RenderWindow& window) override;

private:
    void drawCapsuleButton(sf::RenderWindow& w,
                           float x, float y, float bw, float bh,
                           sf::Color fill, const std::string& label);
    sf::FloatRect m_startBtnBounds;
};