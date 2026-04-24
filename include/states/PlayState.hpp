#pragma once

#include "states/GameState.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include "enemies/Enemy.hpp"
#include "physics/CollisionDetector.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class PlayState : public GameState {
public:
    static const int MAX_PLATFORMS = 16;
    static const int MAX_ENEMIES   = 16;

private:
    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;
    bool m_backgroundLoaded;

    // HUD resources
    sf::Font    m_hudFont;
    bool        m_hudFontLoaded;
    sf::Texture m_heartTexture;
    sf::Texture m_diamondTexture;
    bool        m_heartLoaded;
    bool        m_diamondLoaded;

    // HUD state
    int         m_score;
    int         m_gems;
    int         m_currentLevel;
    int         m_totalLevels;

    // Player spawn position (used for respawn after losing a life)
    sf::Vector2f m_playerSpawn;

    // HUD helper
    void drawHUD(sf::RenderWindow& window);

    sf::Texture m_platformTexture;
    sf::Texture m_platformTopTexture;
    bool m_platformTextureLoaded;
    bool m_platformTopTextureLoaded;
   
Player*   m_player;
Platform* m_platforms[MAX_PLATFORMS];
int       m_platformCount;

// 🔥 KEEP RAYYAN'S ENEMY SYSTEM
Enemy*    m_enemies[MAX_ENEMIES];
int       m_enemyCount;

// 🔥 KEEP POSITION TRACKING (IMPORTANT FOR COLLISION)
float m_playerPrevX;
float m_playerPrevY;
float m_enemyPrevX[MAX_ENEMIES];
float m_enemyPrevY[MAX_ENEMIES];

CollisionDetector m_collider;

bool m_showHitboxes;
bool m_gameOver;

// 🔥 KEEP BOTH FUNCTIONS
void buildLevel();
void spawnEnemies();

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