#pragma once

#include "states/GameState.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include "enemies/Enemy.hpp"
#include "physics/CollisionDetector.hpp"
#include "projectiles/AttackBall.hpp"
#include "effects/HitFlash.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class PlayState : public GameState {
public:
    static const int MAX_PLATFORMS = 16;
    static const int MAX_ENEMIES   = 16;
    static const int MAX_PROJECTILES = 8;

private:
    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;
    bool m_backgroundLoaded;

    // Projectiles (snowballs). Raw pointer array — spec whitelist forbids STL containers.
    AttackBall* m_projectiles[MAX_PROJECTILES];
    int       m_projectileCount;

    // Phase 3: hit flash effects for attack-ball-hits-enemy transitions.
    // Cheap fixed-size ring — spec whitelist forbids STL containers.
    static const int MAX_HIT_FLASHES = 16;
    HitFlash* m_hitFlashes[MAX_HIT_FLASHES];
    int       m_hitFlashCount;


    // Parallel array tracking chain-kill count per rolling enemy index.
    // Kept in sync with m_enemies[] via the GC pass in update().
    int m_chainCount[MAX_ENEMIES];

    
    // cstdlib rand helper for scoring
    int randomScore(int lo, int hi) const;

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

    Enemy*    m_enemies[MAX_ENEMIES];
    int       m_enemyCount;

    float m_playerPrevX;
    float m_playerPrevY;
    float m_enemyPrevX[MAX_ENEMIES];
    float m_enemyPrevY[MAX_ENEMIES];

    CollisionDetector m_collider;

    bool m_showHitboxes;
    bool m_gameOver;   // latches true once enemy contact triggers game over,
                       // so we push GameOverState exactly once.

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