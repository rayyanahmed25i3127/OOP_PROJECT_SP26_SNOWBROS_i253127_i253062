#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
#include "states/GameOverState.hpp"
#include "enemies/Botom.hpp"
#include "enemies/BotomBlue.hpp"
#include "enemies/BotomOrange.hpp"
#include "enemies/FlyngFoogaFoog.hpp"
#include "enemies/FlyngFoogaFoogRed.hpp"
#include "enemies/Tornado.hpp"
#include "enemies/Mogera.hpp"
#include "enemies/MogeraChild.hpp"
#include "projectiles/Knife.hpp"
#include "audio/AudioManager.hpp"
#include "effects/HitFlash.hpp"
#include "powerups/PowerUp.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>

// Leaderboard helpers
std::string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[11];
    sprintf(buffer, "%04d-%02d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
    return std::string(buffer);
}

void saveScore(const std::string& name, int score, int level) {
    std::ofstream file("leaderboard.txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open leaderboard.txt\n";
        return;
    }
    file << name << "," << score << "," << level << "," << getCurrentDate() << "\n";
    file.close();
}

// Globals
Player*    g_player        = nullptr;
Platform** g_platforms     = nullptr;
int        g_platformCount = 0;

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;
    int g_globalDoubleKillEvents = 0;
}

// Constructor
PlayState::PlayState(int characterIndex)
    : m_playerName("")
    , m_characterIndex(characterIndex)
    , m_backgroundTexture()
    , m_backgroundSprite(m_backgroundTexture)
    , m_backgroundLoaded(false)
    , m_projectileCount(0)
    , m_knifeCount(0)
    , m_hitFlashCount(0)
    , m_powerUpCount(0)
    , m_diamondCount(0)
    , m_speedActive(false)
    , m_speedTimer(0.f)
    , m_balloonActive(false)
    , m_balloonTimer(0.f)
    , m_snowballPowerActive(false)
    , m_distanceActive(false)
    , m_displayedType(PowerUp::Type::SpeedBoost)
    , m_hasDisplayed(false)
    , m_puIconSpeedLoaded(false)
    , m_puIconSnowballLoaded(false)
    , m_puIconDistanceLoaded(false)
    , m_puIconBalloonLoaded(false)
    , m_hudFontLoaded(false)
    , m_heartLoaded(false)
    , m_diamondLoaded(false)
    , m_score(0)
    , m_gems(0)
    , m_currentLevel(1)
    , m_totalLevels(10)
    , m_playerSpawn(100.f, 450.f)
    , m_platformTextureLoaded(false)
    , m_platformTopTextureLoaded(false)
    , m_player(nullptr)
    , m_platformCount(0)
    , m_enemyCount(0)
    , m_playerPrevX(0.f)
    , m_playerPrevY(0.f)
    , m_collider(30.f, 770.f)
    , m_showHitboxes(false)
    , m_gameOver(false)
    , m_levelComplete(false)
    , m_levelTransitionTimer(0.f)
    , m_levelSlideOffset(0.f)
    , m_showLevelCompleteText(false)
    , m_bonusDiamondsSpawned(false)
    , m_mogera(nullptr)
    , m_mogeraChildCount(0)
{
    for (int i = 0; i < MAX_HIT_FLASHES; ++i)    m_hitFlashes[i]  = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i)         m_chainCount[i]  = 0;
    for (int i = 0; i < MAX_PROJECTILES; ++i)     m_projectiles[i] = nullptr;
    for (int i = 0; i < MAX_KNIVES; ++i)          m_knives[i] = nullptr;
    for (int i = 0; i < MAX_PLATFORMS; ++i)       m_platforms[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        m_enemies[i]    = nullptr;
        m_enemyPrevX[i] = 0.f;
        m_enemyPrevY[i] = 0.f;
    }
    for (int i = 0; i < MAX_POWERUPS; ++i)  m_powerUps[i] = nullptr;
    for (int i = 0; i < MAX_DIAMONDS; ++i)  m_diamonds[i] = nullptr;
    for (int i = 0; i < MAX_MOGERA_CHILDREN; ++i) {
        m_mogeraChildren[i]   = nullptr;
        m_mogeraChildPrevX[i] = 0.f;
        m_mogeraChildPrevY[i] = 0.f;
    }
}

PlayState::~PlayState() {
    delete m_player; m_player = nullptr;
    for (int i = 0; i < m_platformCount; ++i)  { delete m_platforms[i];   m_platforms[i]   = nullptr; }
    for (int i = 0; i < m_enemyCount; ++i)      { delete m_enemies[i];     m_enemies[i]     = nullptr; }
    for (int i = 0; i < m_projectileCount; ++i) { delete m_projectiles[i]; m_projectiles[i] = nullptr; }
    for (int i = 0; i < m_knifeCount; ++i)      { delete m_knives[i];      m_knives[i]      = nullptr; }
    for (int i = 0; i < m_hitFlashCount; ++i)   { delete m_hitFlashes[i];  m_hitFlashes[i]  = nullptr; }
    for (int i = 0; i < m_powerUpCount; ++i)    { delete m_powerUps[i];    m_powerUps[i]    = nullptr; }
    for (int i = 0; i < m_diamondCount; ++i)    { delete m_diamonds[i];    m_diamonds[i]    = nullptr; }
    delete m_mogera; m_mogera = nullptr;
    for (int i = 0; i < m_mogeraChildCount; ++i) { delete m_mogeraChildren[i]; m_mogeraChildren[i] = nullptr; }
}

void PlayState::onEnter() {
    m_playerName = m_manager->getCurrentUserName();
    g_globalDoubleKillEvents = 0;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::cout << "[PlayState] Entering gameplay\n";

    std::string bgPath = "assets/sprites/bg_lvl1.png";
    if (m_currentLevel == 5)
        bgPath = "assets/sprites/bg_lvl5.png";
    else if (m_currentLevel >= 6 && m_currentLevel <= 9)
        bgPath = "assets/sprites/bg_lvl6.png";

    if (!m_backgroundTexture.loadFromFile(bgPath)) {
        std::cerr << "[PlayState] Could not load " << bgPath << "\n";
        m_backgroundLoaded = m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png");
    } else {
        m_backgroundLoaded = true;
    }
    if (m_backgroundLoaded) {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        m_backgroundSprite.setScale({WINDOW_WIDTH / (float)texSize.x,
                                     WINDOW_HEIGHT / (float)texSize.y});
    }

    m_platformTextureLoaded    = m_platformTexture.loadFromFile("assets/sprites/platform_1.png");
    m_platformTopTextureLoaded = m_platformTopTexture.loadFromFile("assets/sprites/platform_2.png");

    // Restore from Continue save
    {
        PlayerProgress& save = m_manager->getProgress();
        if (save.savedLevel > 0) {
            m_currentLevel = save.savedLevel;
            m_gems         = save.savedGems;
            save.gems      = save.savedGems;
            save.savedLevel = 0;
            save.savedGems  = 0;
            std::cout << "[PlayState] Continuing from level=" << m_currentLevel
                      << " gems=" << m_gems << "\n";
        }
    }

    m_player  = new Player(m_playerSpawn, m_characterIndex);
    g_player  = m_player;

    // Apply pending shop purchases
    {
        PlayerProgress& prog = m_manager->getProgress();
        m_gems = prog.gems;
        if (prog.pendingSpeed)    { activatePowerUp(PowerUp::Type::SpeedBoost);      }
        if (prog.pendingSnowball) { activatePowerUp(PowerUp::Type::SnowballPower);   }
        if (prog.pendingDistance) { activatePowerUp(PowerUp::Type::DistanceIncrease);}
        if (prog.pendingBalloon)  { activatePowerUp(PowerUp::Type::BalloonMode);     }
        if (prog.pendingExtraLifeCount > 0 && m_player) {
            for (int li = 0; li < prog.pendingExtraLifeCount; ++li)
                m_player->addLife();
            std::cout << "[PlayState] Applied " << prog.pendingExtraLifeCount
                      << " Extra Life(s) from shop\n";
        }
        prog.clearPending();
    }

    m_hudFontLoaded  = m_hudFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf");
    m_heartLoaded    = m_heartTexture.loadFromFile("assets/sprites/heart.png");
    m_diamondLoaded  = m_diamondTexture.loadFromFile("assets/sprites/diamond.png");

    loadPowerUpIcons();
    buildLevel();
    g_platforms     = m_platforms;
    g_platformCount = m_platformCount;
    spawnEnemies();
    AudioManager::get().playGameMusic();
}

void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    AudioManager::get().playMenuMusic();

    PlayerProgress& prog = m_manager->getProgress();
    if (!m_gameOver) {
        prog.savedLevel          = m_currentLevel;
        prog.savedGems           = m_gems;
        prog.savedCharacterIndex = m_characterIndex;
        prog.gameOverOccurred    = false;
        prog.gems                = m_gems;
        std::cout << "[PlayState] Saved level=" << m_currentLevel
                  << " gems=" << m_gems << " for continue\n";
    } else {
        prog.savedLevel       = 0;
        prog.savedGems        = 0;
        prog.gameOverOccurred = true;
    }
}

void PlayState::buildLevel() {
    if (!m_platformTextureLoaded) {
        std::cerr << "[PlayState] Skipping platform build — texture missing\n";
        return;
    }

    const float BORDER_W   = 8.5f;
    const float LEFT_EXTRA = 22.f;
    const float p1H = 78.75f;
    const float p2H = 150.f;
    const float p2W = 630.f;

    {
        const sf::Texture& topTex = m_platformTopTextureLoaded ? m_platformTopTexture : m_platformTexture;
        sf::FloatRect lower; lower.position = {30.f, 88.f};    lower.size = {p2W - 60.f,  35.f};
        sf::FloatRect upper; upper.position = {130.f, 45.5f};  upper.size = {p2W - 265.f, 35.f};
        sf::FloatRect boxes[2] = {lower, upper};
        bool solids[2] = {false, true};
        m_platforms[m_platformCount++] = new Platform(topTex, {p2W, p2H}, {(WINDOW_WIDTH - p2W) / 2.f, 49.f}, boxes, 2, solids);
    }
    { float w = 300.f;
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 220.f});
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 220.f}); }
    { float w = 390.f;
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {(WINDOW_WIDTH - w) / 2.f, 329.f}); }
    { float w = 300.f;
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 437.f});
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 437.f}); }
}

void PlayState::spawnEnemies() {
    static const sf::Vector2f UL[] = {
        {60.f,198.f},{120.f,198.f},{180.f,198.f},{240.f,198.f},{300.f,198.f}
    };
    static const sf::Vector2f UR[] = {
        {510.f,198.f},{560.f,198.f},{620.f,198.f},{680.f,198.f},{740.f,198.f}
    };
    static const sf::Vector2f MID[] = {
        {230.f,307.f},{280.f,307.f},{350.f,307.f},{420.f,307.f},{490.f,307.f},{550.f,307.f}
    };
    static const sf::Vector2f TOP[] = {
        {150.f,66.f},{210.f,66.f},{280.f,66.f},{350.f,66.f},{420.f,66.f},{490.f,66.f}
    };

    if (m_currentLevel == 1) {
        m_enemies[m_enemyCount++] = new Botom(UL[0]);
        m_enemies[m_enemyCount++] = new Botom(UR[0]);
        m_enemies[m_enemyCount++] = new Botom(MID[2]);
        m_enemies[m_enemyCount++] = new Botom(TOP[1]);
        m_enemies[m_enemyCount++] = new Botom(TOP[4]);
    }
    else if (m_currentLevel == 2) {
        m_enemies[m_enemyCount++] = new Botom(UL[0]);
        m_enemies[m_enemyCount++] = new Botom(UL[2]);
        m_enemies[m_enemyCount++] = new Botom(UR[0]);
        m_enemies[m_enemyCount++] = new Botom(UR[2]);
        m_enemies[m_enemyCount++] = new Botom(MID[0]);
        m_enemies[m_enemyCount++] = new Botom(MID[2]);
        m_enemies[m_enemyCount++] = new Botom(MID[4]);
        m_enemies[m_enemyCount++] = new Botom(TOP[2]);
    }
    else if (m_currentLevel == 3) {
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(200.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(500.f, 140.f));
        m_enemies[m_enemyCount++] = new Botom(UL[1]);
        m_enemies[m_enemyCount++] = new Botom(UL[3]);
        m_enemies[m_enemyCount++] = new Botom(UR[1]);
        m_enemies[m_enemyCount++] = new Botom(UR[3]);
        m_enemies[m_enemyCount++] = new Botom(MID[1]);
        m_enemies[m_enemyCount++] = new Botom(MID[4]);
    }
    else if (m_currentLevel == 4) {
        std::srand(static_cast<unsigned>(std::time(nullptr)) + 4);
        float xs[] = {80.f,180.f,300.f,420.f,540.f,640.f,120.f,350.f,500.f,660.f};
        float ys[] = {160.f, 270.f, 380.f};
        for (int i = 0; i < 10 && m_diamondCount < MAX_DIAMONDS; ++i) {
            m_diamonds[m_diamondCount] = new Diamond(sf::Vector2f(xs[i], ys[std::rand() % 3]));
            m_diamonds[m_diamondCount]->setBonusDiamond(true);
            m_diamondCount++;
        }
        std::cout << "[PlayState] Level 4 Bonus — spawned 10 diamonds\n";
        m_bonusDiamondsSpawned = true;
    }
    else if (m_currentLevel == 5) {
        m_enemies[m_enemyCount++] = new Botom(UL[0]);
        m_enemies[m_enemyCount++] = new Botom(UL[2]);
        m_enemies[m_enemyCount++] = new Botom(MID[1]);
        m_enemies[m_enemyCount++] = new Botom(TOP[1]);
        m_enemies[m_enemyCount++] = new Botom(TOP[3]);
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(180.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(320.f, 140.f));
        float mX = WINDOW_WIDTH - 8.5f - 170.f - 8.f;
        float mY = 475.f - 200.f;
        m_mogera = new Mogera(sf::Vector2f(mX, mY));
        std::cout << "[PlayState] Level 5 — Mogera spawned at (" << mX << ", " << mY << ")\n";
    }
    else if (m_currentLevel == 6) {
        m_enemies[m_enemyCount++] = new BotomBlue(UL[1]);
        m_enemies[m_enemyCount++] = new BotomBlue(MID[2]);
        m_enemies[m_enemyCount++] = new BotomBlue(UR[2]);
        m_enemies[m_enemyCount++] = new Botom(TOP[1]);
        m_enemies[m_enemyCount++] = new Botom(TOP[4]);
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(180.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(580.f, 140.f));
    }
    else if (m_currentLevel == 7) {
        m_enemies[m_enemyCount++] = new BotomBlue(UL[0]);
        m_enemies[m_enemyCount++] = new BotomBlue(UL[3]);
        m_enemies[m_enemyCount++] = new BotomBlue(UR[1]);
        m_enemies[m_enemyCount++] = new BotomBlue(MID[3]);
        m_enemies[m_enemyCount++] = new BotomOrange(UR[3]);
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(200.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoogRed(sf::Vector2f(500.f, 140.f));
        m_enemies[m_enemyCount++] = new Tornado(TOP[2]);
    }
    else if (m_currentLevel == 8) {
        m_enemies[m_enemyCount++] = new BotomOrange(UL[0]);
        m_enemies[m_enemyCount++] = new BotomOrange(MID[2]);
        m_enemies[m_enemyCount++] = new BotomOrange(UR[4]);
        m_enemies[m_enemyCount++] = new BotomBlue(UL[4]);
        m_enemies[m_enemyCount++] = new BotomBlue(UR[0]);
        m_enemies[m_enemyCount++] = new BotomBlue(TOP[3]);
        m_enemies[m_enemyCount++] = new FlyngFoogaFoogRed(sf::Vector2f(150.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoogRed(sf::Vector2f(450.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(280.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(580.f, 140.f));
        m_enemies[m_enemyCount++] = new Tornado(TOP[1]);
        m_enemies[m_enemyCount++] = new Tornado(TOP[4]);
    }
    else if (m_currentLevel == 9) {
        std::srand(static_cast<unsigned>(std::time(nullptr)) + 9);
        float xs[] = {80.f,180.f,300.f,420.f,540.f,640.f,120.f,350.f,500.f,660.f};
        float ys[] = {160.f, 270.f, 380.f};
        for (int i = 0; i < 10 && m_diamondCount < MAX_DIAMONDS; ++i) {
            m_diamonds[m_diamondCount] = new Diamond(sf::Vector2f(xs[i], ys[std::rand() % 3]));
            m_diamonds[m_diamondCount]->setBonusDiamond(true);
            m_diamondCount++;
        }
        std::cout << "[PlayState] Level 9 Bonus — spawned 10 diamonds\n";
        m_bonusDiamondsSpawned = true;
    }
    else {
        m_enemies[m_enemyCount++] = new Botom(UL[0]);
        m_enemies[m_enemyCount++] = new Botom(UL[2]);
        m_enemies[m_enemyCount++] = new Botom(UR[0]);
        m_enemies[m_enemyCount++] = new Botom(UR[2]);
        m_enemies[m_enemyCount++] = new Botom(MID[1]);
        m_enemies[m_enemyCount++] = new Botom(MID[3]);
        m_enemies[m_enemyCount++] = new Botom(TOP[1]);
        m_enemies[m_enemyCount++] = new Botom(TOP[4]);
    }

    if (m_snowballPowerActive) {
        for (int i = 0; i < m_enemyCount; ++i)
            if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true);
    }

    for (int i = 0; i < m_enemyCount; ++i) {
        if (m_enemies[i]) {
            m_enemyPrevX[i] = m_enemies[i]->getPosition().x;
            m_enemyPrevY[i] = m_enemies[i]->getPosition().y;
        }
    }
    std::cout << "[PlayState] Level " << m_currentLevel
              << " — spawned " << m_enemyCount << " enemies\n";
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (!m_gameOver) m_manager->pushState(new PauseState());
        }
        else if (keyEvent->code == sf::Keyboard::Key::F1
              || keyEvent->code == sf::Keyboard::Key::H) {
            m_showHitboxes = !m_showHitboxes;
            std::cout << "[PlayState] Hitboxes " << (m_showHitboxes ? "ON" : "OFF") << "\n";
        }
    }
}

void PlayState::spawnMogeraChildren(sf::Vector2f mouthPos) {
    struct Arc { float vx; float vy; };
    Arc arcs[3] = { {-340.f,-420.f}, {-340.f,-260.f}, {-340.f,-100.f} };
    for (int i = 0; i < 3; ++i) {
        if (m_mogeraChildCount >= MAX_MOGERA_CHILDREN) break;
        MogeraChild* child = new MogeraChild(mouthPos, {arcs[i].vx, arcs[i].vy});
        m_mogeraChildren[m_mogeraChildCount] = child;
        m_mogeraChildPrevX[m_mogeraChildCount] = mouthPos.x;
        m_mogeraChildPrevY[m_mogeraChildCount] = mouthPos.y;
        m_mogeraChildCount++;
    }
    std::cout << "[PlayState] Spawned 3 MogeraChild babies\n";
}

void PlayState::updateMogera(float dt) {
    if (m_mogera && !m_mogera->isDead()) {
        m_mogera->update(dt);
        Mogera::SpawnRequest req = m_mogera->getAndClearSpawnRequest();
        if (req.pending) spawnMogeraChildren(req.spawnPos);
        if (m_mogera->getAndClearRewardPending()) {
            m_score += 5000;
            for (int i = 0; i < 8 && m_diamondCount < MAX_DIAMONDS; ++i) {
                float dx = m_mogera->getPosition().x - 40.f + (i % 4) * 22.f;
                float dy = m_mogera->getPosition().y - 20.f + (i / 4) * 22.f;
                m_diamonds[m_diamondCount++] = new Diamond(sf::Vector2f(dx, dy));
            }
            std::cout << "[PlayState] Mogera defeated! +5000 score, +8 diamonds\n";
        }
    }

    for (int i = 0; i < m_mogeraChildCount; ++i) {
        MogeraChild* c = m_mogeraChildren[i];
        if (!c) continue;
        m_mogeraChildPrevX[i] = c->getPosition().x;
        m_mogeraChildPrevY[i] = c->getPosition().y;
        c->update(dt);
        if (c->isAlive() && c->getPhase() == MogeraChild::Phase::Ball)
            m_collider.resolve(*c, m_platforms, m_platformCount,
                               m_mogeraChildPrevX[i], m_mogeraChildPrevY[i]);
    }

    // Attack ball vs Mogera
    if (m_mogera && !m_mogera->isDead() && !m_mogera->isDying()) {
        for (int i = 0; i < m_projectileCount; ++i) {
            if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
            sf::FloatRect pH = m_projectiles[i]->getHitBox();
            sf::FloatRect mH = m_mogera->getHitBox();
            bool hit = (pH.position.x + pH.size.x > mH.position.x) &&
                       (pH.position.x < mH.position.x + mH.size.x) &&
                       (pH.position.y + pH.size.y > mH.position.y) &&
                       (pH.position.y < mH.position.y + mH.size.y);
            if (hit) {
                m_mogera->takeSnowballHit();
                m_projectiles[i]->setAlive(false);
                if (m_hitFlashCount < MAX_HIT_FLASHES) {
                    sf::FloatRect r = m_projectiles[i]->getHitBox();
                    m_hitFlashes[m_hitFlashCount++] = new HitFlash(
                        {r.position.x + r.size.x*0.5f - 6.f, r.position.y + r.size.y*0.5f - 8.f});
                }
            }
        }
    }

    // Attack ball vs MogeraChild
    for (int i = 0; i < m_projectileCount; ++i) {
        if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
        sf::FloatRect pH = m_projectiles[i]->getHitBox();
        float pL=pH.position.x, pR=pL+pH.size.x, pT=pH.position.y, pB=pT+pH.size.y;
        for (int c = 0; c < m_mogeraChildCount; ++c) {
            if (!m_mogeraChildren[c] || !m_mogeraChildren[c]->isAlive()) continue;
            sf::FloatRect cH = m_mogeraChildren[c]->getHitBox();
            if (pR>cH.position.x && pL<cH.position.x+cH.size.x &&
                pB>cH.position.y && pT<cH.position.y+cH.size.y) {
                m_mogeraChildren[c]->takeHit();
                m_projectiles[i]->setAlive(false);
                m_score += 100;
                if (m_hitFlashCount < MAX_HIT_FLASHES)
                    m_hitFlashes[m_hitFlashCount++] = new HitFlash({(pL+pR)*0.5f-6.f,(pT+pB)*0.5f-8.f});
                break;
            }
        }
    }

    // Player contact with Mogera
    if (m_player && !m_player->isInvincible() && !m_gameOver
        && m_mogera && m_mogera->canDamagePlayer()) {
        sf::FloatRect pH = m_player->getHitBox();
        sf::FloatRect mH = m_mogera->getHitBox();
        if (pH.position.x+pH.size.x > mH.position.x && pH.position.x < mH.position.x+mH.size.x &&
            pH.position.y+pH.size.y > mH.position.y && pH.position.y < mH.position.y+mH.size.y) {
            m_player->loseLife();
            std::cout << "[PlayState] Player hit by Mogera! Lives: " << m_player->getLives() << "\n";
            if (m_player->getLives() <= 0) {
                m_gameOver = true;
                saveScore(m_manager->getCurrentUserName(), m_score, m_currentLevel);
                m_manager->pushState(new GameOverState());
            } else {
                m_player->respawn(m_playerSpawn);
            }
        }
    }

    // Player contact with MogeraChild (walking phase only)
    if (m_player && !m_player->isInvincible() && !m_gameOver) {
        sf::FloatRect pH = m_player->getHitBox();
        float pL=pH.position.x, pR=pL+pH.size.x, pT=pH.position.y, pB=pT+pH.size.y;
        for (int c = 0; c < m_mogeraChildCount; ++c) {
            if (!m_mogeraChildren[c] || !m_mogeraChildren[c]->isAlive()) continue;
            if (m_mogeraChildren[c]->getPhase() != MogeraChild::Phase::Walking) continue;
            sf::FloatRect cH = m_mogeraChildren[c]->getHitBox();
            if (pR>cH.position.x && pL<cH.position.x+cH.size.x &&
                pB>cH.position.y && pT<cH.position.y+cH.size.y) {
                m_player->loseLife();
                std::cout << "[PlayState] Player hit by MogeraChild! Lives: " << m_player->getLives() << "\n";
                if (m_player->getLives() <= 0) {
                    m_gameOver = true;
                    saveScore(m_manager->getCurrentUserName(), m_score, m_currentLevel);
                    m_manager->pushState(new GameOverState());
                } else {
                    m_player->respawn(m_playerSpawn);
                }
                break;
            }
        }
    }

    // GC dead MogeraChildren
    {
        int write = 0;
        for (int read = 0; read < m_mogeraChildCount; ++read) {
            if (m_mogeraChildren[read] && m_mogeraChildren[read]->isAlive()) {
                m_mogeraChildren[write]   = m_mogeraChildren[read];
                m_mogeraChildPrevX[write] = m_mogeraChildPrevX[read];
                m_mogeraChildPrevY[write] = m_mogeraChildPrevY[read];
                write++;
            } else { delete m_mogeraChildren[read]; m_mogeraChildren[read] = nullptr; }
        }
        m_mogeraChildCount = write;
    }

    if (m_mogera && m_mogera->isDead()) {
        delete m_mogera; m_mogera = nullptr;
        std::cout << "[PlayState] Mogera fully removed\n";
    }
}

void PlayState::update(float dt) {
    // Handle Continue revive
    {
        PlayerProgress& prog = m_manager->getProgress();
        if (prog.pendingRevive && m_player) {
            prog.pendingRevive = false;
            m_gameOver        = false;
            m_player->addLife();
            m_player->respawn(m_playerSpawn);
            m_gems = prog.gems;
            std::cout << "[PlayState] Revived via Continue. Lives="
                      << m_player->getLives() << " Gems=" << m_gems << "\n";
        }
    }

    // Apply pending shop powerups
    {
        PlayerProgress& prog = m_manager->getProgress();
        if (prog.pendingSpeed)    { activatePowerUp(PowerUp::Type::SpeedBoost);       prog.pendingSpeed    = false; }
        if (prog.pendingSnowball) { activatePowerUp(PowerUp::Type::SnowballPower);    prog.pendingSnowball = false; }
        if (prog.pendingDistance) { activatePowerUp(PowerUp::Type::DistanceIncrease); prog.pendingDistance = false; }
        if (prog.pendingBalloon)  { activatePowerUp(PowerUp::Type::BalloonMode);      prog.pendingBalloon  = false; }
        if (prog.pendingExtraLifeCount > 0 && m_player) {
            for (int li = 0; li < prog.pendingExtraLifeCount; ++li)
                m_player->addLife();
            prog.pendingExtraLifeCount = 0;
        }
        m_gems = prog.gems;
    }

    if (m_player) {
        m_playerPrevX = m_player->getPosition().x;
        m_playerPrevY = m_player->getPosition().y;
        m_player->update(dt);
        m_collider.resolve(*m_player, m_platforms, m_platformCount, m_playerPrevX, m_playerPrevY);
    }

    for (int i = 0; i < m_enemyCount; ++i) {
        if (!m_enemies[i]) continue;
        m_enemyPrevX[i] = m_enemies[i]->getPosition().x;
        m_enemyPrevY[i] = m_enemies[i]->getPosition().y;
        m_enemies[i]->update(dt);
        bool skipCollision = (m_enemies[i]->getState() == Enemy::State::Rolling);
        if (!skipCollision) {
            FlyngFoogaFoog* fooga = dynamic_cast<FlyngFoogaFoog*>(m_enemies[i]);
            if (fooga && fooga->isFlying()) skipCollision = true;
        }
        if (!skipCollision) {
            FlyngFoogaFoogRed* foogar = dynamic_cast<FlyngFoogaFoogRed*>(m_enemies[i]);
            if (foogar && foogar->isFlying()) skipCollision = true;
        }
        if (!skipCollision) {
            Tornado* t = dynamic_cast<Tornado*>(m_enemies[i]);
            if (t && t->isFlying()) skipCollision = true;
        }
        if (!skipCollision)
            m_collider.resolve(*m_enemies[i], m_platforms, m_platformCount,
                               m_enemyPrevX[i], m_enemyPrevY[i]);
    }

    if (m_currentLevel == 5) updateMogera(dt);

    // Poll Tornado knife spawn requests
    for (int i = 0; i < m_enemyCount; ++i) {
        Tornado* t = dynamic_cast<Tornado*>(m_enemies[i]);
        if (!t) continue;
        Tornado::KnifeRequest req = t->getAndClearKnifeSpawn();
        if (req.pending && m_knifeCount < MAX_KNIVES)
            m_knives[m_knifeCount++] = new Knife(req.spawnPos, req.direction);
    }

    // Update knives
    for (int i = 0; i < m_knifeCount; ++i)
        if (m_knives[i]) m_knives[i]->update(dt);

    // Knife vs player
    if (m_player && !m_player->isInvincible() && !m_gameOver) {
        sf::FloatRect pH = m_player->getHitBox();
        for (int i = 0; i < m_knifeCount; ++i) {
            if (!m_knives[i] || !m_knives[i]->isAlive()) continue;
            sf::FloatRect kH = m_knives[i]->getHitBox();
            bool hit = (pH.position.x + pH.size.x > kH.position.x) &&
                       (pH.position.x < kH.position.x + kH.size.x) &&
                       (pH.position.y + pH.size.y > kH.position.y) &&
                       (pH.position.y < kH.position.y + kH.size.y);
            if (hit) {
                m_knives[i]->setAlive(false);
                m_player->loseLife();
                std::cout << "[PlayState] Player hit by knife! Lives: " << m_player->getLives() << "\n";
                if (m_player->getLives() <= 0) {
                    m_gameOver = true;
                    saveScore(m_manager->getCurrentUserName(), m_score, m_currentLevel);
                    m_manager->pushState(new GameOverState());
                } else {
                    m_player->respawn(m_playerSpawn);
                }
                break;
            }
        }
    }

    // GC dead knives
    { int w=0; for (int r=0;r<m_knifeCount;++r) {
        if (m_knives[r]&&m_knives[r]->isAlive()) m_knives[w++]=m_knives[r];
        else { delete m_knives[r]; m_knives[r]=nullptr; } } m_knifeCount=w; }

    // Spawn attack ball
    if (m_player && m_player->wantsToThrow() && m_projectileCount < MAX_PROJECTILES) {
        sf::FloatRect pHit = m_player->getHitBox();
        float spawnY = pHit.position.y + pHit.size.y * 0.3f;
        float spawnX = m_player->isFacingRight()
            ? pHit.position.x + pHit.size.x + 2.f
            : pHit.position.x - 16.f - 2.f;
        AttackBall* newBall = new AttackBall({spawnX, spawnY}, m_player->isFacingRight());
        if (m_distanceActive) newBall->setMaxRangeMode(true);
        m_projectiles[m_projectileCount++] = newBall;
        m_player->consumeThrowRequest();
    }

    for (int i = 0; i < m_projectileCount; ++i)
        if (m_projectiles[i]) m_projectiles[i]->update(dt);

    // Attack ball vs regular enemy
    for (int i = 0; i < m_projectileCount; ++i) {
        if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
        sf::FloatRect pHit = m_projectiles[i]->getHitBox();
        float pL=pHit.position.x, pR=pL+pHit.size.x, pT=pHit.position.y, pB=pT+pHit.size.y;
        for (int e = 0; e < m_enemyCount; ++e) {
            if (!m_enemies[e]) continue;
            Enemy::State s = m_enemies[e]->getState();
            if (s != Enemy::State::Alive && s != Enemy::State::PartialEncase &&
                s != Enemy::State::Escaping75 && s != Enemy::State::Escaping50 &&
                s != Enemy::State::Escaping25) continue;
            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            if (pR>eHit.position.x && pL<eHit.position.x+eHit.size.x &&
                pB>eHit.position.y && pT<eHit.position.y+eHit.size.y) {
                m_enemies[e]->takeAttackHit();
                m_projectiles[i]->setAlive(false);
                if (m_hitFlashCount < MAX_HIT_FLASHES)
                    m_hitFlashes[m_hitFlashCount++] = new HitFlash({(pL+pR)*0.5f-6.f,(pT+pB)*0.5f-8.f});
                break;
            }
        }
    }

    // GC dead projectiles
    { int w=0; for (int r=0;r<m_projectileCount;++r) {
        if (m_projectiles[r]&&m_projectiles[r]->isAlive()) m_projectiles[w++]=m_projectiles[r];
        else { delete m_projectiles[r]; m_projectiles[r]=nullptr; } } m_projectileCount=w; }

    // Update & GC hit flashes
    for (int i=0;i<m_hitFlashCount;++i) if (m_hitFlashes[i]) m_hitFlashes[i]->update(dt);
    { int w=0; for (int r=0;r<m_hitFlashCount;++r) {
        if (m_hitFlashes[r]&&m_hitFlashes[r]->isAlive()) m_hitFlashes[w++]=m_hitFlashes[r];
        else { delete m_hitFlashes[r]; m_hitFlashes[r]=nullptr; } } m_hitFlashCount=w; }

    // Player kicks snowballed enemy → Rolling
    if (m_player) {
        sf::FloatRect pHit = m_player->getHitBox();
        float pL=pHit.position.x, pR=pL+pHit.size.x, pT=pHit.position.y, pB=pT+pHit.size.y;
        for (int e=0;e<m_enemyCount;++e) {
            if (!m_enemies[e] || m_enemies[e]->getState()!=Enemy::State::Snowballed) continue;
            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            if (pR>eHit.position.x && pL<eHit.position.x+eHit.size.x &&
                pB>eHit.position.y && pT<eHit.position.y+eHit.size.y) {
                m_enemies[e]->kickIntoRoll(m_player->isFacingRight());
                int ks = randomScore(100,500); m_score+=ks; m_chainCount[e]=1;
                std::cout << "[PlayState] Kicked into roll. +" << ks << " score\n";
                break;
            }
        }
    }

    // Rolling chain kills
    for (int r=0;r<m_enemyCount;++r) {
        if (!m_enemies[r] || m_enemies[r]->getState()!=Enemy::State::Rolling) continue;
        sf::FloatRect rHit = m_enemies[r]->getHitBox();
        float rL=rHit.position.x, rR=rL+rHit.size.x, rT=rHit.position.y, rB=rT+rHit.size.y;
        for (int v=0;v<m_enemyCount;++v) {
            if (v==r||!m_enemies[v]||!m_enemies[v]->isAlive()) continue;
            Enemy::State vs=m_enemies[v]->getState();
            if (vs==Enemy::State::Dead||vs==Enemy::State::Rolling) continue;
            sf::FloatRect vHit=m_enemies[v]->getHitBox();
            if (rR>vHit.position.x && rL<vHit.position.x+vHit.size.x &&
                rB>vHit.position.y && rT<vHit.position.y+vHit.size.y) {
                m_chainCount[r]++;
                int ci=m_chainCount[r];
                if (ci==2) { g_globalDoubleKillEvents++; std::cout<<"[PlayState] Double Kill Event #"<<g_globalDoubleKillEvents<<"\n"; }
                int base=randomScore(100,500), bonus=(int)(base*0.10f*(ci-1));
                m_score+=base+bonus;
                sf::Vector2f sp=m_enemies[v]->getPosition();
                if (m_powerUpCount<MAX_POWERUPS)
                    m_powerUps[m_powerUpCount++]=new PowerUp(sp,static_cast<PowerUp::Type>(std::rand()%static_cast<int>(PowerUp::Type::Count_)));
                if (g_globalDoubleKillEvents%2==0 && m_diamondCount<MAX_DIAMONDS)
                    m_diamonds[m_diamondCount++]=new Diamond(sp);
                m_enemies[v]->setAlive(false);
            }
        }
    }

    // Power-up physics + pickup
    for (int i=0;i<m_powerUpCount;++i) { if (!m_powerUps[i]) continue;
        float px=m_powerUps[i]->getPosition().x, py=m_powerUps[i]->getPosition().y;
        m_powerUps[i]->update(dt);
        m_collider.resolve(*m_powerUps[i],m_platforms,m_platformCount,px,py); }
    if (m_player) {
        sf::FloatRect pH=m_player->getHitBox();
        float pL=pH.position.x,pR=pL+pH.size.x,pT=pH.position.y,pB=pT+pH.size.y;
        for (int i=0;i<m_powerUpCount;++i) { if (!m_powerUps[i]||!m_powerUps[i]->isAlive()) continue;
            sf::FloatRect h=m_powerUps[i]->getHitBox();
            if (pR>h.position.x&&pL<h.position.x+h.size.x&&pB>h.position.y&&pT<h.position.y+h.size.y)
                { activatePowerUp(m_powerUps[i]->getType()); m_powerUps[i]->setAlive(false); } } }
    { int w=0; for (int r=0;r<m_powerUpCount;++r) {
        if (m_powerUps[r]&&m_powerUps[r]->isAlive()) m_powerUps[w++]=m_powerUps[r];
        else { delete m_powerUps[r]; m_powerUps[r]=nullptr; } } m_powerUpCount=w; }

    // Diamond physics + pickup
    for (int i=0;i<m_diamondCount;++i) { if (!m_diamonds[i]) continue;
        float px=m_diamonds[i]->getPosition().x, py=m_diamonds[i]->getPosition().y;
        m_diamonds[i]->update(dt);
        m_collider.resolve(*m_diamonds[i],m_platforms,m_platformCount,px,py); }
    if (m_player) {
        sf::FloatRect pH=m_player->getHitBox();
        float pL=pH.position.x,pR=pL+pH.size.x,pT=pH.position.y,pB=pT+pH.size.y;
        for (int i=0;i<m_diamondCount;++i) { if (!m_diamonds[i]||!m_diamonds[i]->isAlive()) continue;
            sf::FloatRect h=m_diamonds[i]->getHitBox();
            if (pR>h.position.x&&pL<h.position.x+h.size.x&&pB>h.position.y&&pT<h.position.y+h.size.y) {
                m_gems+=Diamond::GEM_VALUE;
                m_manager->getProgress().gems = m_gems;
                m_diamonds[i]->setAlive(false);
                std::cout << "[PlayState] Diamond! +" << Diamond::GEM_VALUE
                          << " gems. Total: " << m_gems << "\n";
            } } }
    { int w=0; for (int r=0;r<m_diamondCount;++r) {
        if (m_diamonds[r]&&m_diamonds[r]->isAlive()) m_diamonds[w++]=m_diamonds[r];
        else { delete m_diamonds[r]; m_diamonds[r]=nullptr; } } m_diamondCount=w; }

    updatePowerUpTimers(dt);

    // GC dead regular enemies
    { int w=0;
      for (int r=0;r<m_enemyCount;++r) {
        Enemy* e=m_enemies[r];
        if (e&&e->isAlive()&&e->getState()!=Enemy::State::Dead)
            { m_enemies[w]=e; m_chainCount[w]=m_chainCount[r]; ++w; }
        else { delete e; m_enemies[r]=nullptr; } }
      for (int i=w;i<m_enemyCount;++i) { m_enemies[i]=nullptr; m_chainCount[i]=0; }
      m_enemyCount=w; }

    // Level Complete Check
    bool levelDone = false;
    if (m_currentLevel == 4 || m_currentLevel == 9)
        levelDone = m_bonusDiamondsSpawned && m_diamondCount==0;
    else if (m_currentLevel == 5)  levelDone = m_enemyCount==0 && m_mogera==nullptr;
    else                           levelDone = m_enemyCount==0;

    if (!m_levelComplete && levelDone) {
        m_levelComplete=true; m_showLevelCompleteText=true; m_levelTransitionTimer=0.f;
        std::cout << "[PlayState] Level " << m_currentLevel << " Complete!\n";
    }

    if (m_levelComplete) {
        m_levelTransitionTimer += dt;
        if (m_levelTransitionTimer < 2.0f) return;
        if (m_levelTransitionTimer < 3.0f) {
            m_showLevelCompleteText=false;
            m_levelSlideOffset=(m_levelTransitionTimer-2.0f)/1.0f*600.f;
            return;
        }
        nextLevel(); return;
    }

    if (!m_gameOver && m_player && !m_player->isInvincible()
        && m_collider.checkEnemyContact(*m_player,m_enemies,m_enemyCount)) {
        m_player->loseLife();
        std::cout << "[PlayState] Player lost a life. Lives left: " << m_player->getLives() << "\n";
        if (m_player->getLives()<=0) {
            m_gameOver=true;
            saveScore(m_manager->getCurrentUserName(), m_score, m_currentLevel);
            m_manager->pushState(new GameOverState());
        }
        else m_player->respawn(m_playerSpawn);
    }
}

void PlayState::draw(sf::RenderWindow& window) {
    if (m_backgroundLoaded) window.draw(m_backgroundSprite);
    else { sf::RectangleShape fb({WINDOW_WIDTH,WINDOW_HEIGHT}); fb.setFillColor(sf::Color(20,30,50)); window.draw(fb); }

    for (int i=0;i<m_platformCount;++i) m_platforms[i]->draw(window);
    for (int i=0;i<m_enemyCount;++i) if (m_enemies[i]) m_enemies[i]->draw(window);
    if (m_mogera) m_mogera->draw(window);
    for (int i=0;i<m_mogeraChildCount;++i) if (m_mogeraChildren[i]) m_mogeraChildren[i]->draw(window);
    for (int i=0;i<m_projectileCount;++i) if (m_projectiles[i]) m_projectiles[i]->draw(window);
    for (int i=0;i<m_knifeCount;++i)     if (m_knives[i])      m_knives[i]->draw(window);
    for (int i=0;i<m_powerUpCount;++i) if (m_powerUps[i]) m_powerUps[i]->draw(window);
    for (int i=0;i<m_diamondCount;++i) if (m_diamonds[i]) m_diamonds[i]->draw(window);
    if (m_player) m_player->draw(window);
    for (int i=0;i<m_hitFlashCount;++i) if (m_hitFlashes[i]) m_hitFlashes[i]->draw(window);

    if (m_showHitboxes) {
        if (m_player) m_player->drawHitBoxDebug(window,sf::Color::Green);
        for (int i=0;i<m_enemyCount;++i) if (m_enemies[i]) m_enemies[i]->drawHitBoxDebug(window,sf::Color::Red);
        if (m_mogera) m_mogera->drawHitBoxDebug(window,sf::Color(255,128,0));
        for (int i=0;i<m_mogeraChildCount;++i) if (m_mogeraChildren[i]) m_mogeraChildren[i]->drawHitBoxDebug(window,sf::Color(255,200,0));
        for (int i=0;i<m_projectileCount;++i) if (m_projectiles[i]) m_projectiles[i]->drawHitBoxDebug(window,sf::Color::Yellow);
        for (int i=0;i<m_powerUpCount;++i) if (m_powerUps[i]) m_powerUps[i]->drawHitBoxDebug(window,sf::Color::Magenta);
        for (int i=0;i<m_diamondCount;++i) if (m_diamonds[i]) m_diamonds[i]->drawHitBoxDebug(window,sf::Color::Cyan);
        for (int i=0;i<m_platformCount;++i) { if (!m_platforms[i]) continue;
            for (int b=0;b<m_platforms[i]->getHitboxCount();++b) {
                sf::FloatRect r=m_platforms[i]->getBounds(b);
                sf::RectangleShape box({r.size.x,r.size.y}); box.setPosition({r.position.x,r.position.y});
                box.setFillColor(sf::Color::Transparent); box.setOutlineColor(sf::Color::Blue); box.setOutlineThickness(1.f);
                window.draw(box); } }
    }

    drawHUD(window);

    if (m_showLevelCompleteText && m_hudFontLoaded) {
        sf::Text t(m_hudFont);
        std::string msg = "Level " + std::to_string(m_currentLevel) + " Complete!";
        if (m_currentLevel==4) msg += "\nGet ready to face the Boss Mogera!";
        else if (m_currentLevel==9) msg += "\nGet ready to face the Boss Gamakichi!";
        t.setString(msg); t.setCharacterSize(40);
        t.setFillColor(sf::Color::Yellow); t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
        sf::FloatRect b=t.getLocalBounds();
        t.setPosition({400.f-b.size.x/2.f, (m_currentLevel==4||m_currentLevel==9)?220.f:250.f});
        window.draw(t);
    }

    if (m_levelSlideOffset > 0.f) {
        sf::RectangleShape overlay({800.f,600.f}); overlay.setFillColor(sf::Color::Black);
        overlay.setPosition({0.f, m_levelSlideOffset-600.f}); window.draw(overlay);
    }
}

void PlayState::drawBossHealthBar(sf::RenderWindow& window) {
    if (!m_mogera || !m_hudFontLoaded) return;
    const float BAR_W=300.f, BAR_H=14.f, BAR_X=(800.f-300.f)/2.f, BAR_Y=32.f;
    int hits=m_mogera->getHitsRemaining(), maxHits=m_mogera->getMaxHits();
    float ratio=(maxHits>0)?(float)hits/(float)maxHits:0.f;

    sf::RectangleShape bg({BAR_W,BAR_H}); bg.setPosition({BAR_X,BAR_Y});
    bg.setFillColor(sf::Color(30,10,10,220)); bg.setOutlineColor(sf::Color(200,100,50)); bg.setOutlineThickness(2.f);
    window.draw(bg);

    if (ratio>0.f) {
        sf::Color fc = ratio>0.5f ? sf::Color(50,200,50) : ratio>0.25f ? sf::Color(220,180,30) : sf::Color(220,50,50);
        sf::RectangleShape fill({BAR_W*ratio,BAR_H}); fill.setPosition({BAR_X,BAR_Y}); fill.setFillColor(fc);
        window.draw(fill);
    }
    sf::Text label(m_hudFont,"MOGERA",9); label.setFillColor(sf::Color(255,160,80));
    label.setOutlineColor(sf::Color::Black); label.setOutlineThickness(1.5f);
    label.setPosition({BAR_X-label.getLocalBounds().size.x-6.f, BAR_Y+1.f}); window.draw(label);

    sf::Text hp(m_hudFont,std::to_string(hits)+"/"+std::to_string(maxHits),9);
    hp.setFillColor(sf::Color::White); hp.setOutlineColor(sf::Color::Black); hp.setOutlineThickness(1.5f);
    hp.setPosition({BAR_X+BAR_W+6.f,BAR_Y+1.f}); window.draw(hp);
}

void PlayState::drawHUD(sf::RenderWindow& window) {
    if (!m_hudFontLoaded) return;
    const float HUD_Y=14.f, LEFT_X=18.f, RIGHT_PAD=18.f, ICON_SIZE=16.f, GAP=6.f;
    const unsigned int TS=14;

    { sf::Text t(m_hudFont,"SCORE "+std::to_string(m_score),TS);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      t.setPosition({LEFT_X,HUD_Y}); window.draw(t); }

    { float ly=HUD_Y+22.f; int lives=m_player?m_player->getLives():0;
      if (m_heartLoaded) { sf::Sprite h(m_heartTexture); auto ts=m_heartTexture.getSize();
        if (ts.x>0&&ts.y>0) h.setScale({ICON_SIZE/(float)ts.x,ICON_SIZE/(float)ts.y});
        h.setPosition({LEFT_X,ly}); window.draw(h); }
      sf::Text t(m_hudFont,std::to_string(lives),TS);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      t.setPosition({LEFT_X+ICON_SIZE+GAP,ly}); window.draw(t); }

    { sf::Text t(m_hudFont,std::to_string(m_gems),TS);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      auto tb=t.getLocalBounds(); float tx=800.f-RIGHT_PAD-tb.size.x-tb.position.x;
      t.setPosition({tx,HUD_Y}); window.draw(t);
      if (m_diamondLoaded) { sf::Sprite d(m_diamondTexture); auto ts=m_diamondTexture.getSize();
        if (ts.x>0&&ts.y>0) d.setScale({ICON_SIZE/(float)ts.x,ICON_SIZE/(float)ts.y});
        d.setPosition({tx-ICON_SIZE-GAP,HUD_Y}); window.draw(d); } }

    { std::string ls="LEVEL "+std::to_string(m_currentLevel)+"/"+std::to_string(m_totalLevels);
      sf::Text t(m_hudFont,ls,TS); t.setFillColor(sf::Color::White);
      t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      auto tb=t.getLocalBounds(); t.setPosition({(800.f-tb.size.x)/2.f-tb.position.x,HUD_Y});
      window.draw(t); }

    if (m_currentLevel==5 && m_mogera) drawBossHealthBar(window);

    if (m_player && m_player->isAutoAttack() && m_hudFontLoaded) {
        sf::Text t(m_hudFont, "AUTO", 10);
        t.setFillColor(sf::Color(80, 255, 80));
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(1.5f);
        t.setPosition({LEFT_X, HUD_Y + 44.f});
        window.draw(t);
    }

    drawPowerUpHUD(window);
}

int PlayState::randomScore(int lo, int hi) const {
    return lo + (std::rand() % (hi - lo + 1));
}

void PlayState::loadPowerUpIcons() {
    auto tryLoad = [](sf::Texture& tex, const char* path, bool& flag) {
        if (std::FILE* f=std::fopen(path,"rb")) { std::fclose(f); flag=tex.loadFromFile(path); }
        else flag=false;
    };
    tryLoad(m_puIconSpeed,    "assets/sprites/powerup_speed.png",    m_puIconSpeedLoaded);
    tryLoad(m_puIconSnowball, "assets/sprites/powerup_snowball.png", m_puIconSnowballLoaded);
    tryLoad(m_puIconDistance, "assets/sprites/powerup_distance.png", m_puIconDistanceLoaded);
    tryLoad(m_puIconBalloon,  "assets/sprites/powerup_balloon.png",  m_puIconBalloonLoaded);
}

void PlayState::activatePowerUp(PowerUp::Type type) {
    switch (type) {
        case PowerUp::Type::SpeedBoost:
            m_speedActive=true; m_speedTimer=15.0f;
            if (m_player) m_player->setSpeedMultiplier(1.5f); break;
        case PowerUp::Type::SnowballPower:
            m_snowballPowerActive=true;
            for (int i=0;i<m_enemyCount;++i) if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true); break;
        case PowerUp::Type::DistanceIncrease:
            m_distanceActive=true; break;
        case PowerUp::Type::BalloonMode:
            m_balloonActive=true; m_balloonTimer=10.0f;
            if (m_player) m_player->setBalloonMode(true); break;
        default: return;
    }
    m_displayedType=type; m_hasDisplayed=true;
    std::cout << "[PlayState] Activated power-up: " << PowerUp::typeName(type) << "\n";
}

void PlayState::updatePowerUpTimers(float dt) {
    if (m_speedActive) {
        m_speedTimer-=dt;
        if (m_speedTimer<=0.f) {
            m_speedActive=false; m_speedTimer=0.f;
            if (m_player) m_player->setSpeedMultiplier(1.0f);
            if (m_hasDisplayed&&m_displayedType==PowerUp::Type::SpeedBoost) {
                if (m_balloonActive) m_displayedType=PowerUp::Type::BalloonMode;
                else if (m_snowballPowerActive) m_displayedType=PowerUp::Type::SnowballPower;
                else if (m_distanceActive) m_displayedType=PowerUp::Type::DistanceIncrease;
                else m_hasDisplayed=false;
            }
        }
    }
    if (m_balloonActive) {
        m_balloonTimer-=dt;
        if (m_balloonTimer<=0.f) {
            m_balloonActive=false; m_balloonTimer=0.f;
            if (m_player) m_player->setBalloonMode(false);
            if (m_hasDisplayed&&m_displayedType==PowerUp::Type::BalloonMode) {
                if (m_speedActive) m_displayedType=PowerUp::Type::SpeedBoost;
                else if (m_snowballPowerActive) m_displayedType=PowerUp::Type::SnowballPower;
                else if (m_distanceActive) m_displayedType=PowerUp::Type::DistanceIncrease;
                else m_hasDisplayed=false;
            }
        }
    }
}

void PlayState::drawPowerUpHUD(sf::RenderWindow& window) {
    if (!m_hasDisplayed) return;
    const float BY=568.f, IS=22.f, BW=160.f, BH=10.f, GAP=8.f;
    const float totalW=IS+GAP+BW, startX=(800.f-totalW)/2.f;
    const float barX=startX+IS+GAP, barY=BY+(IS-BH)/2.f;

    sf::Texture* tex=nullptr; bool loaded=false;
    float fillRatio=1.0f; sf::Color barColor=sf::Color::White;
    switch (m_displayedType) {
        case PowerUp::Type::SpeedBoost:     tex=&m_puIconSpeed;    loaded=m_puIconSpeedLoaded;    fillRatio=m_speedTimer<=0.f?0.f:m_speedTimer/15.f;   barColor={120,200,255}; break;
        case PowerUp::Type::SnowballPower:  tex=&m_puIconSnowball; loaded=m_puIconSnowballLoaded; fillRatio=1.f; barColor={255,120,200}; break;
        case PowerUp::Type::DistanceIncrease: tex=&m_puIconDistance; loaded=m_puIconDistanceLoaded; fillRatio=1.f; barColor={80,140,255}; break;
        case PowerUp::Type::BalloonMode:    tex=&m_puIconBalloon;  loaded=m_puIconBalloonLoaded;  fillRatio=m_balloonTimer<=0.f?0.f:m_balloonTimer/10.f; barColor={180,220,255}; break;
        default: return;
    }
    if (loaded&&tex) { sf::Sprite icon(*tex); auto ts=tex->getSize();
        if (ts.x>0&&ts.y>0) icon.setScale({IS/(float)ts.x,IS/(float)ts.y});
        icon.setPosition({startX,BY}); window.draw(icon); }

    sf::RectangleShape bg({BW,BH}); bg.setPosition({barX,barY});
    bg.setFillColor(sf::Color(20,20,30,200)); bg.setOutlineColor(sf::Color::Black); bg.setOutlineThickness(1.f);
    window.draw(bg);
    if (fillRatio>0.f) { sf::RectangleShape fill({BW*fillRatio,BH}); fill.setPosition({barX,barY});
        fill.setFillColor(barColor); window.draw(fill); }
}

void PlayState::nextLevel() {
    cleanupLevel();
    m_currentLevel++;
    if (m_currentLevel > m_totalLevels) {
        std::cout << "[PlayState] All levels complete! Returning to main menu.\n";
        m_manager->popState();
        return;
    }

    m_levelComplete=false; m_levelTransitionTimer=0.f; m_levelSlideOffset=0.f;
    m_showLevelCompleteText=false; m_bonusDiamondsSpawned=false;

    std::string bgPath = "assets/sprites/bg_lvl1.png";
    if (m_currentLevel == 5) bgPath = "assets/sprites/bg_lvl5.png";
    else if (m_currentLevel >= 6 && m_currentLevel <= 9) bgPath = "assets/sprites/bg_lvl6.png";

    bool bgLoaded = false;
    if (std::FILE* f=std::fopen(bgPath.c_str(),"rb")) {
        std::fclose(f);
        bgLoaded = m_backgroundTexture.loadFromFile(bgPath);
        if (!bgLoaded) std::cerr << "[PlayState] loadFromFile FAILED: " << bgPath << "\n";
    } else {
        std::cerr << "[PlayState] FILE NOT FOUND: " << bgPath << "\n";
        bgLoaded = m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png");
    }
    m_backgroundLoaded = bgLoaded;
    if (m_backgroundLoaded) {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto sz=m_backgroundTexture.getSize();
        m_backgroundSprite.setScale({WINDOW_WIDTH/(float)sz.x, WINDOW_HEIGHT/(float)sz.y});
    }

    buildLevel();
    g_platforms=m_platforms; g_platformCount=m_platformCount;
    spawnEnemies();
    if (m_player) m_player->respawn(m_playerSpawn);
    std::cout << "[PlayState] Started Level " << m_currentLevel << "\n";
}

void PlayState::cleanupLevel() {
    for (int i=0;i<m_platformCount;++i)  { delete m_platforms[i];   m_platforms[i]=nullptr;   } m_platformCount=0;
    for (int i=0;i<m_enemyCount;++i)     { delete m_enemies[i];     m_enemies[i]=nullptr; m_chainCount[i]=0; } m_enemyCount=0;
    for (int i=0;i<m_projectileCount;++i){ delete m_projectiles[i]; m_projectiles[i]=nullptr; } m_projectileCount=0;
    for (int i=0;i<m_knifeCount;++i)    { delete m_knives[i];      m_knives[i]=nullptr;      } m_knifeCount=0;
    for (int i=0;i<m_powerUpCount;++i)   { delete m_powerUps[i];    m_powerUps[i]=nullptr;    } m_powerUpCount=0;
    for (int i=0;i<m_diamondCount;++i)   { delete m_diamonds[i];    m_diamonds[i]=nullptr;    } m_diamondCount=0;
    for (int i=0;i<m_hitFlashCount;++i)  { delete m_hitFlashes[i];  m_hitFlashes[i]=nullptr;  } m_hitFlashCount=0;
    delete m_mogera; m_mogera=nullptr;
    for (int i=0;i<m_mogeraChildCount;++i){ delete m_mogeraChildren[i]; m_mogeraChildren[i]=nullptr; } m_mogeraChildCount=0;
}