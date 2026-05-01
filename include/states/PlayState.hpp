#pragma once

#include "states/GameState.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include "enemies/Enemy.hpp"
#include "physics/CollisionDetector.hpp"
#include "projectiles/AttackBall.hpp"
#include "effects/HitFlash.hpp"
#include "powerups/PowerUp.hpp"
#include "powerups/Diamond.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class PlayState : public GameState {
    //making leaderboard functional:
    std::string m_playerName;
    
public:
    static const int MAX_PLATFORMS = 16;
    static const int MAX_ENEMIES   = 16;
    static const int MAX_PROJECTILES = 8;
    int m_characterIndex;

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

    // --- Power-ups (spec §8) ---
    static const int MAX_POWERUPS = 16;
    PowerUp* m_powerUps[MAX_POWERUPS];
    int      m_powerUpCount;

    // --- Diamonds (collectible gems) ---
    static const int MAX_DIAMONDS = 16;
    Diamond* m_diamonds[MAX_DIAMONDS];
    int      m_diamondCount;

    // Active player effects. Snowball Power & Distance Increase are
    // "until level end" so timers stay ≥0 forever once activated;
    // Speed (15s) and Balloon (10s) drain to zero.
    bool  m_speedActive;       float m_speedTimer;
    bool  m_balloonActive;     float m_balloonTimer;
    bool  m_snowballPowerActive;     // permanent for the level
    bool  m_distanceActive;          // permanent for the level

    // The currently-displayed icon at the bottom HUD. We only show ONE
    // active power-up at a time (the most recently activated one). If
    // its timer expires we fall back to next-active or none.
    PowerUp::Type m_displayedType;
    bool          m_hasDisplayed;

    // Cached HUD textures for power-up icons (load-on-demand)
    sf::Texture m_puIconSpeed;     bool m_puIconSpeedLoaded;
    sf::Texture m_puIconSnowball;  bool m_puIconSnowballLoaded;
    sf::Texture m_puIconDistance;  bool m_puIconDistanceLoaded;
    sf::Texture m_puIconBalloon;   bool m_puIconBalloonLoaded;

    void activatePowerUp(PowerUp::Type type);
    void updatePowerUpTimers(float dt);
    void drawPowerUpHUD(sf::RenderWindow& window);
    void loadPowerUpIcons();

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
   PlayState(int characterIndex=0);
    ~PlayState();

    PlayState(const PlayState&) = delete;
    PlayState& operator=(const PlayState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
    void onExit() override;

    // ===== Power-up query helpers =====
    /**
     * @brief Check if Speed Boost power-up is currently active
     */
    bool isSpeedActive() const { return m_speedActive; }

    /**
     * @brief Check if Distance Increase power-up is currently active
     */
    bool isDistanceActive() const { return m_distanceActive; }
};