#pragma once

#include "states/GameState.hpp"
#include "Leaderboard.hpp"
#include <SFML/Graphics.hpp>

class LeaderboardState : public GameState {
private:
    sf::Texture bgTexture;
    sf::Sprite* bgSprite  = nullptr;   // raw pointer â€” no optional
    sf::Text*   backText  = nullptr;   // raw pointer â€” no optional

    sf::Font       font;
    Leaderboard    leaderboard;
    bool           isBackHovered = false;

    // Draws a smooth capsule as a single TriangleFan â€” no vector, no seam.
    // outlineColor / outlineThickness are optional params â€” default = no outline.
    void drawCapsule(sf::RenderWindow& window,
                     float x, float y,
                     float width, float height,
                     sf::Color fillColor,
                     sf::Color outlineColor    = sf::Color::Transparent,
                     float    outlineThickness = 0.f);

public:
    LeaderboardState();
    ~LeaderboardState();              // deletes bgSprite and backText

    void onEnter()                           override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
};