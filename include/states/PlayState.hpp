#pragma once

#include "states/GameState.hpp"
#include "Player.hpp"
#include "Platform.hpp"
#include "enemies/Enemy.hpp"
#include "enemies/Mogera.hpp"
#include "enemies/MogeraChild.hpp"
#include "physics/CollisionDetector.hpp"
#include "projectiles/AttackBall.hpp"
#include "effects/HitFlash.hpp"
#include "powerups/PowerUp.hpp"
#include "powerups/Diamond.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class PlayState : public GameState {
public:
    static const int MAX_PLATFORMS       = 16;
    static const int MAX_ENEMIES         = 16;
    static const int MAX_PROJECTILES     = 8;
    static const int MAX_HIT_FLASHES     = 16;
    static const int MAX_POWERUPS        = 16;
    static const int MAX_DIAMONDS        = 16;
    static const int MAX_MOGERA_CHILDREN = 32;

private:
    std::string m_playerName;
    int m_characterIndex;

    sf::Texture m_backgroundTexture;
    sf::Sprite  m_backgroundSprite;
    bool m_backgroundLoaded;

    AttackBall* m_projectiles[MAX_PROJECTILES];
    int         m_projectileCount;

    HitFlash* m_hitFlashes[MAX_HIT_FLASHES];
    int       m_hitFlashCount;

    PowerUp* m_powerUps[MAX_POWERUPS];
    int      m_powerUpCount;

    Diamond* m_diamonds[MAX_DIAMONDS];
    int      m_diamondCount;

    bool  m_speedActive;
    float m_speedTimer;
    bool  m_balloonActive;
    float m_balloonTimer;
    bool  m_snowballPowerActive;
    bool  m_distanceActive;

    PowerUp::Type m_displayedType;
    bool          m_hasDisplayed;

    sf::Texture m_puIconSpeed;
    bool        m_puIconSpeedLoaded;
    sf::Texture m_puIconSnowball;
    bool        m_puIconSnowballLoaded;
    sf::Texture m_puIconDistance;
    bool        m_puIconDistanceLoaded;
    sf::Texture m_puIconBalloon;
    bool        m_puIconBalloonLoaded;

    void activatePowerUp(PowerUp::Type type);
    void updatePowerUpTimers(float dt);
    void drawPowerUpHUD(sf::RenderWindow& window);
    void loadPowerUpIcons();

    int m_chainCount[MAX_ENEMIES];
    int randomScore(int lo, int hi) const;

    sf::Font    m_hudFont;
    bool        m_hudFontLoaded;
    sf::Texture m_heartTexture;
    sf::Texture m_diamondTexture;
    bool        m_heartLoaded;
    bool        m_diamondLoaded;

    int         m_score;
    int         m_gems;
    int         m_currentLevel;
    int         m_totalLevels;

    sf::Vector2f m_playerSpawn;

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
    bool m_gameOver;

    bool  m_levelComplete;
    float m_levelTransitionTimer;
    float m_levelSlideOffset;
    bool  m_showLevelCompleteText;
    bool  m_bonusDiamondsSpawned;

    // ===== BOSS: Mogera (Level 5) =====
    Mogera*      m_mogera;
    MogeraChild* m_mogeraChildren[MAX_MOGERA_CHILDREN];
    int          m_mogeraChildCount;
    float        m_mogeraChildPrevX[MAX_MOGERA_CHILDREN];
    float        m_mogeraChildPrevY[MAX_MOGERA_CHILDREN];

    void drawBossHealthBar(sf::RenderWindow& window);
    void spawnMogeraChildren(sf::Vector2f mouthPos);
    void updateMogera(float dt);
    // ======================================

    void buildLevel();
    void spawnEnemies();
    void nextLevel();
    void cleanupLevel();

public:
    PlayState(int characterIndex = 0);
    ~PlayState();

    PlayState(const PlayState&) = delete;
    PlayState& operator=(const PlayState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
    void onExit() override;

    bool isSpeedActive()    const { return m_speedActive;    }
    bool isDistanceActive() const { return m_distanceActive; }
};