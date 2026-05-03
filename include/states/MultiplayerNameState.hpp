#pragma once
#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <string>

// Simple name-entry screen shown before a multiplayer game starts.
// Player 1 name = logged-in username (read-only).
// Player 2 name = typed by the user here.
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