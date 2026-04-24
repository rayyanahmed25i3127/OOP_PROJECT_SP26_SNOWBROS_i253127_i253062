#pragma once

#include "states/GameState.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include "physics/CollisionDetector.hpp"
#include <SFML/Graphics.hpp>

class PlayState : public GameState {
public:
    static const int MAX_PLATFORMS = 16;

private:
    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;
    bool m_backgroundLoaded;

    sf::Texture m_platformTexture;
    sf::Texture m_platformTopTexture;
    bool m_platformTextureLoaded;
    bool m_platformTopTextureLoaded;
   

    Player* m_player;
    Platform* m_platforms[MAX_PLATFORMS];
    int m_platformCount;
    

    // New: collision system
    CollisionDetector m_collider;

    // New: track previous Y so collider can distinguish landing-on vs jumping-through
    float m_playerPrevY;

    bool m_showHitboxes;

    void buildLevel();
    

public:
    PlayState();
    ~PlayState();

    PlayState(const PlayState&) = delete;
    PlayState& operator=(const PlayState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
    void onExit() override;
};