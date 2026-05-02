#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
#include "states/GameOverState.hpp"
#include "enemies/Botom.hpp"
#include "enemies/FlyngFoogaFoog.hpp"
#include "enemies/Mogera.hpp"
#include "enemies/MogeraChild.hpp"
#include "audio/AudioManager.hpp"
#include "effects/HitFlash.hpp"
#include "powerups/PowerUp.hpp"
#include "powerups/Diamond.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

Player*    g_player        = nullptr;
Platform** g_platforms     = nullptr;
int        g_platformCount = 0;

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;
    int g_globalDoubleKillEvents = 0;
}

// ---------------------------------------------------------------
PlayState::PlayState()
    : m_backgroundSprite(m_backgroundTexture)
    , m_backgroundLoaded(false)
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
    , m_hudFontLoaded(false)
    , m_heartLoaded(false)
    , m_diamondLoaded(false)
    , m_score(0)
    , m_gems(0)
    , m_currentLevel(1)
    , m_totalLevels(10)
    , m_playerSpawn(100.f, 450.f)
    , m_projectileCount(0)
    , m_hitFlashCount(0)
    , m_powerUpCount(0)
    , m_diamondCount(0)
    , m_speedActive(false), m_speedTimer(0.f)
    , m_balloonActive(false), m_balloonTimer(0.f)
    , m_snowballPowerActive(false)
    , m_distanceActive(false)
    , m_displayedType(PowerUp::Type::SpeedBoost)
    , m_hasDisplayed(false)
    , m_puIconSpeedLoaded(false)
    , m_puIconSnowballLoaded(false)
    , m_puIconDistanceLoaded(false)
    , m_puIconBalloonLoaded(false)
    // Boss
    , m_mogera(nullptr)
    , m_mogeraChildCount(0)
{
    for (int i = 0; i < MAX_HIT_FLASHES; ++i)    m_hitFlashes[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i)         m_chainCount[i] = 0;
    for (int i = 0; i < MAX_PROJECTILES; ++i)     m_projectiles[i] = nullptr;
    for (int i = 0; i < MAX_PLATFORMS; ++i)       m_platforms[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        m_enemies[i]    = nullptr;
        m_enemyPrevX[i] = 0.f;
        m_enemyPrevY[i] = 0.f;
    }
    for (int i = 0; i < MAX_POWERUPS; ++i)  m_powerUps[i] = nullptr;
    for (int i = 0; i < MAX_DIAMONDS; ++i)  m_diamonds[i] = nullptr;

    for (int i = 0; i < MAX_MOGERA_CHILDREN; ++i) {
        m_mogeraChildren[i]    = nullptr;
        m_mogeraChildPrevX[i]  = 0.f;
        m_mogeraChildPrevY[i]  = 0.f;
    }
}

// ---------------------------------------------------------------
PlayState::~PlayState() {
    delete m_player;
    m_player = nullptr;

    for (int i = 0; i < m_platformCount; ++i) {
        delete m_platforms[i]; m_platforms[i] = nullptr;
    }
    for (int i = 0; i < m_enemyCount; ++i) {
        delete m_enemies[i]; m_enemies[i] = nullptr;
    }
    for (int i = 0; i < m_projectileCount; ++i) {
        delete m_projectiles[i]; m_projectiles[i] = nullptr;
    }
    for (int i = 0; i < m_hitFlashCount; ++i) {
        delete m_hitFlashes[i]; m_hitFlashes[i] = nullptr;
    }
    for (int i = 0; i < m_powerUpCount; ++i) {
        delete m_powerUps[i]; m_powerUps[i] = nullptr;
    }
    for (int i = 0; i < m_diamondCount; ++i) {
        delete m_diamonds[i]; m_diamonds[i] = nullptr;
    }
    delete m_mogera; m_mogera = nullptr;
    for (int i = 0; i < m_mogeraChildCount; ++i) {
        delete m_mogeraChildren[i]; m_mogeraChildren[i] = nullptr;
    }
}

// ---------------------------------------------------------------
void PlayState::onEnter() {
    g_globalDoubleKillEvents = 0;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::cout << "[PlayState] Entering gameplay\n";

    // Background loaded for the current level (fallback to lvl1)
    std::string bgPath = "assets/sprites/bg_lvl1.png";
    if (m_currentLevel == 5) bgPath = "assets/sprites/bg_lvl5.png";

    if (!m_backgroundTexture.loadFromFile(bgPath)) {
        std::cerr << "[PlayState] Could not load " << bgPath << "\n";
        // Try fallback
        m_backgroundLoaded = m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png");
    } else {
        m_backgroundLoaded = true;
    }
    if (m_backgroundLoaded) {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_backgroundSprite.setScale({scaleX, scaleY});
    }

    if (!m_platformTexture.loadFromFile("assets/sprites/platform_1.png"))
        m_platformTextureLoaded = false;
    else
        m_platformTextureLoaded = true;

    if (!m_platformTopTexture.loadFromFile("assets/sprites/platform_2.png"))
        m_platformTopTextureLoaded = false;
    else
        m_platformTopTextureLoaded = true;

    m_player = new Player(m_playerSpawn);
    g_player = m_player;

    if (!m_hudFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf"))
        m_hudFontLoaded = false;
    else
        m_hudFontLoaded = true;

    if (!m_heartTexture.loadFromFile("assets/sprites/heart.png"))
        m_heartLoaded = false;
    else
        m_heartLoaded = true;

    if (!m_diamondTexture.loadFromFile("assets/sprites/diamond.png"))
        m_diamondLoaded = false;
    else
        m_diamondLoaded = true;

    loadPowerUpIcons();
    buildLevel();
    g_platforms     = m_platforms;
    g_platformCount = m_platformCount;
    spawnEnemies();

    AudioManager::get().playGameMusic();
}

// ---------------------------------------------------------------
void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    AudioManager::get().playMenuMusic();
}

// ---------------------------------------------------------------
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

    // Top-center stepped platform (multi-hitbox)
    {
        const sf::Texture& topTex = m_platformTopTextureLoaded
            ? m_platformTopTexture : m_platformTexture;
        sf::Vector2f p2Size = { p2W, p2H };
        sf::Vector2f p2Pos  = { (WINDOW_WIDTH - p2W) / 2.f, 49.f };
        sf::FloatRect lower;
        lower.position = { 30.f, 88.f };
        lower.size     = { p2W - 60.f, 35.f };
        sf::FloatRect upper;
        upper.position = { 130.f, 45.5f };
        upper.size     = { p2W - 265.f, 35.f };
        sf::FloatRect boxes[2] = { lower, upper };
        bool solids[2]         = { false, true };
        m_platforms[m_platformCount++] =
            new Platform(topTex, p2Size, p2Pos, boxes, 2, solids);
    }

    // Upper L & R
    {
        float w = 300.f;
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 220.f});
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 220.f});
    }

    // Mid-center
    {
        float w = 390.f;
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {(WINDOW_WIDTH - w) / 2.f, 329.f});
    }

    // Lower L & R
    {
        float w = 300.f;
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 437.f});
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 437.f});
    }
}

// ---------------------------------------------------------------
void PlayState::spawnEnemies() {
    if (m_currentLevel == 1) {
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(140.f, 400.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(600.f, 400.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(300.f, 290.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(150.f, 180.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(620.f, 180.f));
    }
    else if (m_currentLevel == 2) {
        std::srand(static_cast<unsigned>(std::time(nullptr)) + m_currentLevel);
        float positions[] = {140.f, 300.f, 450.f, 600.f, 150.f, 350.f, 500.f, 650.f};
        float yLevels[]   = {400.f, 290.f, 180.f};
        for (int i = 0; i < 8; ++i)
            m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(positions[i], yLevels[std::rand() % 3]));
    }
    else if (m_currentLevel == 3) {
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(200.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(500.f, 140.f));
        std::srand(static_cast<unsigned>(std::time(nullptr)) + m_currentLevel);
        float positions[] = {140.f, 300.f, 450.f, 600.f, 150.f, 500.f};
        float yLevels[]   = {400.f, 290.f, 180.f};
        for (int i = 0; i < 6; ++i)
            m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(positions[i], yLevels[std::rand() % 3]));
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
        // --- BOSS LEVEL 5 ---
        // 5 Botoms on various platforms
        float botomX[] = {80.f, 250.f, 205.f, 150.f, 310.f};
        float botomY[] = {400.f, 400.f, 290.f, 180.f, 180.f};
        for (int i = 0; i < 5; ++i)
            m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(botomX[i], botomY[i]));

        // 2 FlyngFoogaFoog on top platform
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(180.f, 140.f));
        m_enemies[m_enemyCount++] = new FlyngFoogaFoog(sf::Vector2f(320.f, 140.f));

        // --- Mogera boss: lower-right platform ---
        // Lower-right platform surface is at y = 457 (pos 437 + inset 20).
        // Mogera's feet sit on that surface: position.y = 457 - spriteH.
        float mX = WINDOW_WIDTH - 8.5f - 170.f - 8.f;   // flush near right wall (170px wide)
        float mY = 475.f - 200.f;                         // feet on platform surface (200px tall)
        m_mogera = new Mogera(sf::Vector2f(mX, mY));
        std::cout << "[PlayState] Level 5 — Mogera spawned at (" << mX << ", " << mY << ")\n";
    }
    else {
        // Placeholder levels 6-10
        std::srand(static_cast<unsigned>(std::time(nullptr)) + m_currentLevel);
        float positions[] = {140.f, 300.f, 450.f, 600.f, 150.f, 350.f, 500.f, 650.f};
        float yLevels[]   = {400.f, 290.f, 180.f};
        for (int i = 0; i < 8; ++i)
            m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(positions[i], yLevels[std::rand() % 3]));
    }

    if (m_snowballPowerActive) {
        for (int i = 0; i < m_enemyCount; ++i)
            if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true);
    }
    std::cout << "[PlayState] Level " << m_currentLevel
              << " — spawned " << m_enemyCount << " regular enemies\n";
}

// ---------------------------------------------------------------
void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (!m_gameOver)
                m_manager->pushState(new PauseState());
        }
        else if (keyEvent->code == sf::Keyboard::Key::F1
              || keyEvent->code == sf::Keyboard::Key::H) {
            m_showHitboxes = !m_showHitboxes;
            std::cout << "[PlayState] Hitboxes " << (m_showHitboxes ? "ON" : "OFF") << "\n";
        }
    }
}

// ---------------------------------------------------------------
// spawnMogeraChildren — called when Mogera signals a spawn request
// Throws 3 babies in a slight spread of arcs from the mouth position.
// ---------------------------------------------------------------
void PlayState::spawnMogeraChildren(sf::Vector2f mouthPos) {
    // Three babies: top arc, mid arc, low arc
    // All travel left (-x). Vary the initial y-velocity for spread.
    struct Arc { float vx; float vy; };
    Arc arcs[3] = {
        { -340.f, -420.f },   // high arc
        { -340.f, -260.f },   // mid arc
        { -340.f, -100.f },   // low arc (nearly straight)
    };

    for (int i = 0; i < 3; ++i) {
        if (m_mogeraChildCount >= MAX_MOGERA_CHILDREN) break;
        MogeraChild* child = new MogeraChild(mouthPos, { arcs[i].vx, arcs[i].vy });
        m_mogeraChildren[m_mogeraChildCount] = child;
        m_mogeraChildPrevX[m_mogeraChildCount] = mouthPos.x;
        m_mogeraChildPrevY[m_mogeraChildCount] = mouthPos.y;
        m_mogeraChildCount++;
    }
    std::cout << "[PlayState] Spawned 3 MogeraChild babies\n";
}

// ---------------------------------------------------------------
// updateMogera — all Mogera + MogeraChild update/collision logic
// Called from the main update() when on Level 5.
// ---------------------------------------------------------------
void PlayState::updateMogera(float dt) {
    // --- Update Mogera boss ---
    if (m_mogera && !m_mogera->isDead()) {
        m_mogera->update(dt);

        // Poll spawn request
        Mogera::SpawnRequest req = m_mogera->getAndClearSpawnRequest();
        if (req.pending) {
            spawnMogeraChildren(req.spawnPos);
        }

        // Poll reward
        if (m_mogera->getAndClearRewardPending()) {
            m_score += 5000;
            // Drop 8 diamonds (each = 15 gems → total 120 gems)
            for (int i = 0; i < 8 && m_diamondCount < MAX_DIAMONDS; ++i) {
                float dx = m_mogera->getPosition().x - 40.f + (i % 4) * 22.f;
                float dy = m_mogera->getPosition().y - 20.f + (i / 4) * 22.f;
                m_diamonds[m_diamondCount++] = new Diamond(sf::Vector2f(dx, dy));
            }
            std::cout << "[PlayState] Mogera defeated! +5000 score, +8 diamonds (120 gems)\n";
        }
    }

    // --- Update MogeraChild array ---
    for (int i = 0; i < m_mogeraChildCount; ++i) {
        MogeraChild* c = m_mogeraChildren[i];
        if (!c) continue;

        m_mogeraChildPrevX[i] = c->getPosition().x;
        m_mogeraChildPrevY[i] = c->getPosition().y;

        c->update(dt);

        // Collider runs for Ball phase only so babies land on platforms.
        // Walking babies MUST skip the collider — it clamps to the left wall
        // (x=30) and zeroes velocity.x, which stacks every walker at the edge.
        // Ground-keeping for walkers is handled inside MogeraChild::update().
        if (c->isAlive() && c->getPhase() == MogeraChild::Phase::Ball) {
            m_collider.resolve(*c, m_platforms, m_platformCount,
                               m_mogeraChildPrevX[i], m_mogeraChildPrevY[i]);
        }
    }

    // --- Attack ball vs Mogera ---
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
                // Hit flash
                if (m_hitFlashCount < MAX_HIT_FLASHES) {
                    sf::FloatRect r = m_projectiles[i]->getHitBox();
                    sf::Vector2f fp{ r.position.x + r.size.x * 0.5f - 6.f,
                                     r.position.y + r.size.y * 0.5f - 8.f };
                    m_hitFlashes[m_hitFlashCount++] = new HitFlash(fp);
                }
            }
        }
    }

    // --- Attack ball vs MogeraChild ---
    for (int i = 0; i < m_projectileCount; ++i) {
        if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
        sf::FloatRect pH = m_projectiles[i]->getHitBox();
        float pL = pH.position.x, pR = pL + pH.size.x;
        float pT = pH.position.y, pB = pT + pH.size.y;

        for (int c = 0; c < m_mogeraChildCount; ++c) {
            if (!m_mogeraChildren[c] || !m_mogeraChildren[c]->isAlive()) continue;
            sf::FloatRect cH = m_mogeraChildren[c]->getHitBox();
            bool hit = (pR > cH.position.x) && (pL < cH.position.x + cH.size.x) &&
                       (pB > cH.position.y) && (pT < cH.position.y + cH.size.y);
            if (hit) {
                m_mogeraChildren[c]->takeHit();
                m_projectiles[i]->setAlive(false);
                m_score += 100;
                if (m_hitFlashCount < MAX_HIT_FLASHES) {
                    sf::Vector2f fp{ (pL + pR) * 0.5f - 6.f, (pT + pB) * 0.5f - 8.f };
                    m_hitFlashes[m_hitFlashCount++] = new HitFlash(fp);
                }
                break;
            }
        }
    }

    // --- Player contact with Mogera body ---
    if (m_player && !m_player->isInvincible() && !m_gameOver
        && m_mogera && m_mogera->canDamagePlayer()) {
        sf::FloatRect pH = m_player->getHitBox();
        sf::FloatRect mH = m_mogera->getHitBox();
        bool hit = (pH.position.x + pH.size.x > mH.position.x) &&
                   (pH.position.x < mH.position.x + mH.size.x) &&
                   (pH.position.y + pH.size.y > mH.position.y) &&
                   (pH.position.y < mH.position.y + mH.size.y);
        if (hit) {
            m_player->loseLife();
            std::cout << "[PlayState] Player hit by Mogera! Lives: " << m_player->getLives() << "\n";
            if (m_player->getLives() <= 0) {
                m_gameOver = true;
                m_manager->pushState(new GameOverState());
            } else {
                m_player->respawn(m_playerSpawn);
            }
        }
    }

    // --- Player contact with MogeraChild ---
    if (m_player && !m_player->isInvincible() && !m_gameOver) {
        sf::FloatRect pH = m_player->getHitBox();
        float pL = pH.position.x, pR = pL + pH.size.x;
        float pT = pH.position.y, pB = pT + pH.size.y;

        for (int c = 0; c < m_mogeraChildCount; ++c) {
            if (!m_mogeraChildren[c] || !m_mogeraChildren[c]->isAlive()) continue;
            // Only walking babies damage player (ball phase is just physics)
            if (m_mogeraChildren[c]->getPhase() != MogeraChild::Phase::Walking) continue;

            sf::FloatRect cH = m_mogeraChildren[c]->getHitBox();
            bool hit = (pR > cH.position.x) && (pL < cH.position.x + cH.size.x) &&
                       (pB > cH.position.y) && (pT < cH.position.y + cH.size.y);
            if (hit) {
                m_player->loseLife();
                std::cout << "[PlayState] Player hit by MogeraChild! Lives: "
                          << m_player->getLives() << "\n";
                if (m_player->getLives() <= 0) {
                    m_gameOver = true;
                    m_manager->pushState(new GameOverState());
                } else {
                    m_player->respawn(m_playerSpawn);
                }
                break;
            }
        }
    }

    // --- GC dead MogeraChildren ---
    {
        int write = 0;
        for (int read = 0; read < m_mogeraChildCount; ++read) {
            if (m_mogeraChildren[read] && m_mogeraChildren[read]->isAlive()) {
                m_mogeraChildren[write]   = m_mogeraChildren[read];
                m_mogeraChildPrevX[write] = m_mogeraChildPrevX[read];
                m_mogeraChildPrevY[write] = m_mogeraChildPrevY[read];
                write++;
            } else {
                delete m_mogeraChildren[read];
                m_mogeraChildren[read] = nullptr;
            }
        }
        m_mogeraChildCount = write;
    }

    // --- GC dead Mogera ---
    if (m_mogera && m_mogera->isDead()) {
        delete m_mogera;
        m_mogera = nullptr;
        std::cout << "[PlayState] Mogera fully removed\n";
    }
}

// ---------------------------------------------------------------
void PlayState::update(float dt) {
    // --- Player ---
    if (m_player) {
        m_playerPrevX = m_player->getPosition().x;
        m_playerPrevY = m_player->getPosition().y;
        m_player->update(dt);
        m_collider.resolve(*m_player, m_platforms, m_platformCount,
                           m_playerPrevX, m_playerPrevY);
    }

    // --- Regular Enemies ---
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
            m_collider.resolve(*m_enemies[i], m_platforms, m_platformCount,
                               m_enemyPrevX[i], m_enemyPrevY[i]);
        }
    }

    // --- Mogera boss update (Level 5) ---
    if (m_currentLevel == 5) {
        updateMogera(dt);
    }

    // --- Spawn attack ball ---
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

    // --- Update projectiles ---
    for (int i = 0; i < m_projectileCount; ++i)
        if (m_projectiles[i]) m_projectiles[i]->update(dt);

    // --- Attack ball vs regular enemy collision ---
    for (int i = 0; i < m_projectileCount; ++i) {
        if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
        sf::FloatRect pHit = m_projectiles[i]->getHitBox();
        float pL = pHit.position.x, pR = pL + pHit.size.x;
        float pT = pHit.position.y, pB = pT + pHit.size.y;

        for (int e = 0; e < m_enemyCount; ++e) {
            if (!m_enemies[e]) continue;
            Enemy::State s = m_enemies[e]->getState();
            if (s != Enemy::State::Alive &&
                s != Enemy::State::PartialEncase &&
                s != Enemy::State::Escaping75 &&
                s != Enemy::State::Escaping50 &&
                s != Enemy::State::Escaping25) continue;

            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            bool overlap = (pR > eHit.position.x) && (pL < eHit.position.x + eHit.size.x) &&
                           (pB > eHit.position.y) && (pT < eHit.position.y + eHit.size.y);
            if (!overlap) continue;

            m_enemies[e]->takeAttackHit();
            m_projectiles[i]->setAlive(false);
            if (m_hitFlashCount < MAX_HIT_FLASHES) {
                sf::Vector2f fp{ (pL + pR) * 0.5f - 6.f, (pT + pB) * 0.5f - 8.f };
                m_hitFlashes[m_hitFlashCount++] = new HitFlash(fp);
            }
            break;
        }
    }

    // --- GC dead projectiles ---
    {
        int write = 0;
        for (int read = 0; read < m_projectileCount; ++read) {
            if (m_projectiles[read] && m_projectiles[read]->isAlive()) {
                m_projectiles[write++] = m_projectiles[read];
            } else {
                delete m_projectiles[read];
                m_projectiles[read] = nullptr;
            }
        }
        m_projectileCount = write;
    }

    // --- Update & GC hit flashes ---
    for (int i = 0; i < m_hitFlashCount; ++i)
        if (m_hitFlashes[i]) m_hitFlashes[i]->update(dt);
    {
        int write = 0;
        for (int read = 0; read < m_hitFlashCount; ++read) {
            if (m_hitFlashes[read] && m_hitFlashes[read]->isAlive()) {
                m_hitFlashes[write++] = m_hitFlashes[read];
            } else {
                delete m_hitFlashes[read];
                m_hitFlashes[read] = nullptr;
            }
        }
        m_hitFlashCount = write;
    }

    // --- Player kicks snowballed enemy → Rolling ---
    if (m_player) {
        sf::FloatRect pHit = m_player->getHitBox();
        float pL = pHit.position.x, pR = pL + pHit.size.x;
        float pT = pHit.position.y, pB = pT + pHit.size.y;
        for (int e = 0; e < m_enemyCount; ++e) {
            if (!m_enemies[e] || m_enemies[e]->getState() != Enemy::State::Snowballed) continue;
            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            bool overlap = (pR > eHit.position.x) && (pL < eHit.position.x + eHit.size.x) &&
                           (pB > eHit.position.y) && (pT < eHit.position.y + eHit.size.y);
            if (!overlap) continue;
            m_enemies[e]->kickIntoRoll(m_player->isFacingRight());
            int kickedScore = randomScore(100, 500);
            m_score += kickedScore;
            m_chainCount[e] = 1;
            std::cout << "[PlayState] Kicked into roll. +" << kickedScore << " score\n";
            break;
        }
    }

    // --- Rolling enemy chain kills ---
    for (int r = 0; r < m_enemyCount; ++r) {
        if (!m_enemies[r] || m_enemies[r]->getState() != Enemy::State::Rolling) continue;
        sf::FloatRect rHit = m_enemies[r]->getHitBox();
        float rL = rHit.position.x, rR = rL + rHit.size.x;
        float rT = rHit.position.y, rB = rT + rHit.size.y;

        for (int v = 0; v < m_enemyCount; ++v) {
            if (v == r || !m_enemies[v] || !m_enemies[v]->isAlive()) continue;
            Enemy::State vs = m_enemies[v]->getState();
            if (vs == Enemy::State::Dead || vs == Enemy::State::Rolling) continue;
            sf::FloatRect vHit = m_enemies[v]->getHitBox();
            bool overlap = (rR > vHit.position.x) && (rL < vHit.position.x + vHit.size.x) &&
                           (rB > vHit.position.y) && (rT < vHit.position.y + vHit.size.y);
            if (!overlap) continue;

            m_chainCount[r]++;
            int chainIndex = m_chainCount[r];
            if (chainIndex == 2) {
                g_globalDoubleKillEvents++;
                std::cout << "[PlayState] Double Kill Event #" << g_globalDoubleKillEvents << "\n";
            }

            int base  = randomScore(100, 500);
            int bonus = static_cast<int>(base * 0.10f * (chainIndex - 1));
            m_score  += base + bonus;

            sf::Vector2f spawnPos = m_enemies[v]->getPosition();
            if (m_powerUpCount < MAX_POWERUPS) {
                int typeIdx = std::rand() % static_cast<int>(PowerUp::Type::Count_);
                m_powerUps[m_powerUpCount++] =
                    new PowerUp(spawnPos, static_cast<PowerUp::Type>(typeIdx));
            }
            if (g_globalDoubleKillEvents % 2 == 0 && m_diamondCount < MAX_DIAMONDS)
                m_diamonds[m_diamondCount++] = new Diamond(spawnPos);

            m_enemies[v]->setAlive(false);
        }
    }

    // --- Power-up physics + pickup ---
    for (int i = 0; i < m_powerUpCount; ++i) {
        if (!m_powerUps[i]) continue;
        float prevX = m_powerUps[i]->getPosition().x;
        float prevY = m_powerUps[i]->getPosition().y;
        m_powerUps[i]->update(dt);
        m_collider.resolve(*m_powerUps[i], m_platforms, m_platformCount, prevX, prevY);
    }
    if (m_player) {
        sf::FloatRect pH = m_player->getHitBox();
        float pL = pH.position.x, pR = pL + pH.size.x;
        float pT = pH.position.y, pB = pT + pH.size.y;
        for (int i = 0; i < m_powerUpCount; ++i) {
            if (!m_powerUps[i] || !m_powerUps[i]->isAlive()) continue;
            sf::FloatRect h = m_powerUps[i]->getHitBox();
            bool overlap = (pR > h.position.x) && (pL < h.position.x + h.size.x) &&
                           (pB > h.position.y) && (pT < h.position.y + h.size.y);
            if (!overlap) continue;
            activatePowerUp(m_powerUps[i]->getType());
            m_powerUps[i]->setAlive(false);
        }
    }
    {
        int write = 0;
        for (int read = 0; read < m_powerUpCount; ++read) {
            if (m_powerUps[read] && m_powerUps[read]->isAlive())
                m_powerUps[write++] = m_powerUps[read];
            else { delete m_powerUps[read]; m_powerUps[read] = nullptr; }
        }
        m_powerUpCount = write;
    }

    // --- Diamond physics + pickup ---
    for (int i = 0; i < m_diamondCount; ++i) {
        if (!m_diamonds[i]) continue;
        float prevX = m_diamonds[i]->getPosition().x;
        float prevY = m_diamonds[i]->getPosition().y;
        m_diamonds[i]->update(dt);
        m_collider.resolve(*m_diamonds[i], m_platforms, m_platformCount, prevX, prevY);
    }
    if (m_player) {
        sf::FloatRect pH = m_player->getHitBox();
        float pL = pH.position.x, pR = pL + pH.size.x;
        float pT = pH.position.y, pB = pT + pH.size.y;
        for (int i = 0; i < m_diamondCount; ++i) {
            if (!m_diamonds[i] || !m_diamonds[i]->isAlive()) continue;
            sf::FloatRect h = m_diamonds[i]->getHitBox();
            bool overlap = (pR > h.position.x) && (pL < h.position.x + h.size.x) &&
                           (pB > h.position.y) && (pT < h.position.y + h.size.y);
            if (!overlap) continue;
            m_gems += Diamond::GEM_VALUE;
            std::cout << "[PlayState] Diamond! +" << Diamond::GEM_VALUE << " gems. Total: " << m_gems << "\n";
            m_diamonds[i]->setAlive(false);
        }
    }
    {
        int write = 0;
        for (int read = 0; read < m_diamondCount; ++read) {
            if (m_diamonds[read] && m_diamonds[read]->isAlive())
                m_diamonds[write++] = m_diamonds[read];
            else { delete m_diamonds[read]; m_diamonds[read] = nullptr; }
        }
        m_diamondCount = write;
    }

    updatePowerUpTimers(dt);

    // --- GC dead regular enemies ---
    {
        int write = 0;
        for (int read = 0; read < m_enemyCount; ++read) {
            Enemy* e = m_enemies[read];
            bool keep = e && e->isAlive() && e->getState() != Enemy::State::Dead;
            if (keep) {
                m_enemies[write]    = e;
                m_chainCount[write] = m_chainCount[read];
                ++write;
            } else {
                delete e;
                m_enemies[read] = nullptr;
            }
        }
        for (int i = write; i < m_enemyCount; ++i) {
            m_enemies[i]    = nullptr;
            m_chainCount[i] = 0;
        }
        m_enemyCount = write;
    }

    // --- Level Complete Check ---
    bool levelDone = false;
    if (m_currentLevel == 4) {
        levelDone = (m_bonusDiamondsSpawned && m_diamondCount == 0);
    } else if (m_currentLevel == 5) {
        // Level 5 done when: all regular enemies gone AND Mogera is gone
        levelDone = (m_enemyCount == 0) && (m_mogera == nullptr);
    } else {
        levelDone = (m_enemyCount == 0);
    }

    if (!m_levelComplete && levelDone) {
        m_levelComplete = true;
        m_showLevelCompleteText = true;
        m_levelTransitionTimer  = 0.f;
        std::cout << "[PlayState] Level " << m_currentLevel << " Complete!\n";
    }

    // --- Level Transition ---
    if (m_levelComplete) {
        m_levelTransitionTimer += dt;
        if (m_levelTransitionTimer < 2.0f) return;
        if (m_levelTransitionTimer < 3.0f) {
            m_showLevelCompleteText = false;
            float progress = (m_levelTransitionTimer - 2.0f) / 1.0f;
            m_levelSlideOffset = progress * 600.f;
            return;
        } else {
            nextLevel();
            return;
        }
    }

    // --- Player-Enemy contact ---
    if (!m_gameOver && m_player && !m_player->isInvincible()
        && m_collider.checkEnemyContact(*m_player, m_enemies, m_enemyCount)) {
        m_player->loseLife();
        std::cout << "[PlayState] Player lost a life. Lives left: " << m_player->getLives() << "\n";
        if (m_player->getLives() <= 0) {
            m_gameOver = true;
            m_manager->pushState(new GameOverState());
        } else {
            m_player->respawn(m_playerSpawn);
        }
    }
}

// ---------------------------------------------------------------
void PlayState::draw(sf::RenderWindow& window) {
    if (m_backgroundLoaded) {
        window.draw(m_backgroundSprite);
    } else {
        sf::RectangleShape fallback({WINDOW_WIDTH, WINDOW_HEIGHT});
        fallback.setFillColor(sf::Color(20, 30, 50));
        window.draw(fallback);
    }

    for (int i = 0; i < m_platformCount; ++i)
        m_platforms[i]->draw(window);

    for (int i = 0; i < m_enemyCount; ++i)
        if (m_enemies[i]) m_enemies[i]->draw(window);

    // Draw Mogera boss
    if (m_mogera) m_mogera->draw(window);

    // Draw MogeraChildren
    for (int i = 0; i < m_mogeraChildCount; ++i)
        if (m_mogeraChildren[i]) m_mogeraChildren[i]->draw(window);

    for (int i = 0; i < m_projectileCount; ++i)
        if (m_projectiles[i]) m_projectiles[i]->draw(window);

    for (int i = 0; i < m_powerUpCount; ++i)
        if (m_powerUps[i]) m_powerUps[i]->draw(window);

    for (int i = 0; i < m_diamondCount; ++i)
        if (m_diamonds[i]) m_diamonds[i]->draw(window);

    if (m_player) m_player->draw(window);

    for (int i = 0; i < m_hitFlashCount; ++i)
        if (m_hitFlashes[i]) m_hitFlashes[i]->draw(window);

    // Debug hitboxes
    if (m_showHitboxes) {
        if (m_player) m_player->drawHitBoxDebug(window, sf::Color::Green);
        for (int i = 0; i < m_enemyCount; ++i)
            if (m_enemies[i]) m_enemies[i]->drawHitBoxDebug(window, sf::Color::Red);
        if (m_mogera) m_mogera->drawHitBoxDebug(window, sf::Color(255, 128, 0));
        for (int i = 0; i < m_mogeraChildCount; ++i)
            if (m_mogeraChildren[i]) m_mogeraChildren[i]->drawHitBoxDebug(window, sf::Color(255, 200, 0));
        for (int i = 0; i < m_projectileCount; ++i)
            if (m_projectiles[i]) m_projectiles[i]->drawHitBoxDebug(window, sf::Color::Yellow);
        for (int i = 0; i < m_powerUpCount; ++i)
            if (m_powerUps[i]) m_powerUps[i]->drawHitBoxDebug(window, sf::Color::Magenta);
        for (int i = 0; i < m_diamondCount; ++i)
            if (m_diamonds[i]) m_diamonds[i]->drawHitBoxDebug(window, sf::Color::Cyan);
        for (int i = 0; i < m_platformCount; ++i) {
            if (!m_platforms[i]) continue;
            for (int b = 0; b < m_platforms[i]->getHitboxCount(); ++b) {
                sf::FloatRect r = m_platforms[i]->getBounds(b);
                sf::RectangleShape box({r.size.x, r.size.y});
                box.setPosition({r.position.x, r.position.y});
                box.setFillColor(sf::Color::Transparent);
                box.setOutlineColor(sf::Color::Blue);
                box.setOutlineThickness(1.f);
                window.draw(box);
            }
        }
    }

    drawHUD(window);

    // Level Complete text
    if (m_showLevelCompleteText && m_hudFontLoaded) {
        sf::Text t(m_hudFont);
        std::string msg = "Level " + std::to_string(m_currentLevel) + " Complete!";
        if (m_currentLevel == 4) msg += "\nGet ready to face the Boss Mogera!";
        else if (m_currentLevel == 9) msg += "\nGet ready to face the Boss Gamakichi!";
        t.setString(msg);
        t.setCharacterSize(40);
        t.setFillColor(sf::Color::Yellow);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(2.f);
        sf::FloatRect b = t.getLocalBounds();
        float yOff = (m_currentLevel == 4 || m_currentLevel == 9) ? 220.f : 250.f;
        t.setPosition({400.f - b.size.x / 2.f, yOff});
        window.draw(t);
    }

    // Slide transition overlay
    if (m_levelSlideOffset > 0.f) {
        sf::RectangleShape overlay({800.f, 600.f});
        overlay.setFillColor(sf::Color::Black);
        overlay.setPosition({0.f, m_levelSlideOffset - 600.f});
        window.draw(overlay);
    }
}

// ---------------------------------------------------------------
void PlayState::drawBossHealthBar(sf::RenderWindow& window) {
    if (!m_mogera) return;
    if (!m_hudFontLoaded) return;

    const float BAR_W      = 300.f;
    const float BAR_H      = 14.f;
    const float BAR_X      = (800.f - BAR_W) / 2.f;
    const float BAR_Y      = 32.f;   // just below level indicator line

    int hits     = m_mogera->getHitsRemaining();
    int maxHits  = m_mogera->getMaxHits();
    float ratio  = (maxHits > 0) ? (static_cast<float>(hits) / static_cast<float>(maxHits)) : 0.f;

    // Background
    sf::RectangleShape bg({BAR_W, BAR_H});
    bg.setPosition({BAR_X, BAR_Y});
    bg.setFillColor(sf::Color(30, 10, 10, 220));
    bg.setOutlineColor(sf::Color(200, 100, 50));
    bg.setOutlineThickness(2.f);
    window.draw(bg);

    // Health fill — gradient green→yellow→red
    if (ratio > 0.f) {
        sf::Color fillColor;
        if (ratio > 0.5f)       fillColor = sf::Color(50, 200, 50);
        else if (ratio > 0.25f) fillColor = sf::Color(220, 180, 30);
        else                    fillColor = sf::Color(220, 50, 50);

        sf::RectangleShape fill({BAR_W * ratio, BAR_H});
        fill.setPosition({BAR_X, BAR_Y});
        fill.setFillColor(fillColor);
        window.draw(fill);
    }

    // Label: "MOGERA" left of bar
    sf::Text label(m_hudFont, "MOGERA", 9);
    label.setFillColor(sf::Color(255, 160, 80));
    label.setOutlineColor(sf::Color::Black);
    label.setOutlineThickness(1.5f);
    sf::FloatRect lb = label.getLocalBounds();
    label.setPosition({BAR_X - lb.size.x - 6.f, BAR_Y + 1.f});
    window.draw(label);

    // HP number right of bar
    sf::Text hpText(m_hudFont, std::to_string(hits) + "/" + std::to_string(maxHits), 9);
    hpText.setFillColor(sf::Color::White);
    hpText.setOutlineColor(sf::Color::Black);
    hpText.setOutlineThickness(1.5f);
    hpText.setPosition({BAR_X + BAR_W + 6.f, BAR_Y + 1.f});
    window.draw(hpText);
}

// ---------------------------------------------------------------
void PlayState::drawHUD(sf::RenderWindow& window) {
    if (!m_hudFontLoaded) return;

    const float HUD_Y       = 14.f;
    const float LEFT_X      = 18.f;
    const float RIGHT_PAD   = 18.f;
    const float ICON_SIZE   = 16.f;
    const float ICON_TEXT_GAP = 6.f;
    const unsigned int TEXT_SIZE = 14;

    // Score
    {
        sf::Text t(m_hudFont, "SCORE " + std::to_string(m_score), TEXT_SIZE);
        t.setFillColor(sf::Color::White);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(2.f);
        t.setPosition({LEFT_X, HUD_Y});
        window.draw(t);
    }

    // Lives
    {
        float livesY = HUD_Y + 22.f;
        int lives = (m_player ? m_player->getLives() : 0);
        if (m_heartLoaded) {
            sf::Sprite heart(m_heartTexture);
            auto ts = m_heartTexture.getSize();
            if (ts.x > 0 && ts.y > 0)
                heart.setScale({ICON_SIZE / (float)ts.x, ICON_SIZE / (float)ts.y});
            heart.setPosition({LEFT_X, livesY});
            window.draw(heart);
        }
        sf::Text t(m_hudFont, std::to_string(lives), TEXT_SIZE);
        t.setFillColor(sf::Color::White);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(2.f);
        t.setPosition({LEFT_X + ICON_SIZE + ICON_TEXT_GAP, livesY});
        window.draw(t);
    }

    // Gems
    {
        std::string gemStr = std::to_string(m_gems);
        sf::Text t(m_hudFont, gemStr, TEXT_SIZE);
        t.setFillColor(sf::Color::White);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(2.f);
        auto tb = t.getLocalBounds();
        float textX = 800.f - RIGHT_PAD - tb.size.x - tb.position.x;
        t.setPosition({textX, HUD_Y});
        window.draw(t);
        if (m_diamondLoaded) {
            sf::Sprite d(m_diamondTexture);
            auto ts = m_diamondTexture.getSize();
            if (ts.x > 0 && ts.y > 0)
                d.setScale({ICON_SIZE / (float)ts.x, ICON_SIZE / (float)ts.y});
            d.setPosition({textX - ICON_SIZE - ICON_TEXT_GAP, HUD_Y});
            window.draw(d);
        }
    }

    // Level indicator
    {
        std::string levelStr = "LEVEL " + std::to_string(m_currentLevel) +
                               "/" + std::to_string(m_totalLevels);
        sf::Text t(m_hudFont, levelStr, TEXT_SIZE);
        t.setFillColor(sf::Color::White);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(2.f);
        auto tb = t.getLocalBounds();
        t.setPosition({(800.f - tb.size.x) / 2.f - tb.position.x, HUD_Y});
        window.draw(t);
    }

    // Boss health bar (Level 5)
    if (m_currentLevel == 5 && m_mogera) {
        drawBossHealthBar(window);
    }

    drawPowerUpHUD(window);
}

// ---------------------------------------------------------------
int PlayState::randomScore(int lo, int hi) const {
    int range = hi - lo + 1;
    return lo + (std::rand() % range);
}

// ---------------------------------------------------------------
void PlayState::loadPowerUpIcons() {
    auto tryLoad = [](sf::Texture& tex, const char* path, bool& flag) {
        if (std::FILE* f = std::fopen(path, "rb")) {
            std::fclose(f);
            flag = tex.loadFromFile(path);
        } else { flag = false; }
    };
    tryLoad(m_puIconSpeed,    "assets/sprites/powerup_speed.png",    m_puIconSpeedLoaded);
    tryLoad(m_puIconSnowball, "assets/sprites/powerup_snowball.png", m_puIconSnowballLoaded);
    tryLoad(m_puIconDistance, "assets/sprites/powerup_distance.png", m_puIconDistanceLoaded);
    tryLoad(m_puIconBalloon,  "assets/sprites/powerup_balloon.png",  m_puIconBalloonLoaded);
}

// ---------------------------------------------------------------
void PlayState::activatePowerUp(PowerUp::Type type) {
    switch (type) {
        case PowerUp::Type::SpeedBoost:
            m_speedActive = true; m_speedTimer = 15.0f;
            if (m_player) m_player->setSpeedMultiplier(1.5f);
            break;
        case PowerUp::Type::SnowballPower:
            m_snowballPowerActive = true;
            for (int i = 0; i < m_enemyCount; ++i)
                if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true);
            break;
        case PowerUp::Type::DistanceIncrease:
            m_distanceActive = true;
            break;
        case PowerUp::Type::BalloonMode:
            m_balloonActive = true; m_balloonTimer = 10.0f;
            if (m_player) m_player->setBalloonMode(true);
            break;
        default: return;
    }
    m_displayedType = type;
    m_hasDisplayed  = true;
    std::cout << "[PlayState] Activated power-up: " << PowerUp::typeName(type) << "\n";
}

// ---------------------------------------------------------------
void PlayState::updatePowerUpTimers(float dt) {
    if (m_speedActive) {
        m_speedTimer -= dt;
        if (m_speedTimer <= 0.f) {
            m_speedActive = false; m_speedTimer = 0.f;
            if (m_player) m_player->setSpeedMultiplier(1.0f);
            if (m_hasDisplayed && m_displayedType == PowerUp::Type::SpeedBoost) {
                if      (m_balloonActive)       m_displayedType = PowerUp::Type::BalloonMode;
                else if (m_snowballPowerActive) m_displayedType = PowerUp::Type::SnowballPower;
                else if (m_distanceActive)      m_displayedType = PowerUp::Type::DistanceIncrease;
                else                            m_hasDisplayed  = false;
            }
        }
    }
    if (m_balloonActive) {
        m_balloonTimer -= dt;
        if (m_balloonTimer <= 0.f) {
            m_balloonActive = false; m_balloonTimer = 0.f;
            if (m_player) m_player->setBalloonMode(false);
            if (m_hasDisplayed && m_displayedType == PowerUp::Type::BalloonMode) {
                if      (m_speedActive)         m_displayedType = PowerUp::Type::SpeedBoost;
                else if (m_snowballPowerActive) m_displayedType = PowerUp::Type::SnowballPower;
                else if (m_distanceActive)      m_displayedType = PowerUp::Type::DistanceIncrease;
                else                            m_hasDisplayed  = false;
            }
        }
    }
}

// ---------------------------------------------------------------
void PlayState::drawPowerUpHUD(sf::RenderWindow& window) {
    if (!m_hasDisplayed) return;

    const float HUD_BOTTOM_Y = 568.f;
    const float ICON_SIZE    = 22.f;
    const float BAR_WIDTH    = 160.f;
    const float BAR_HEIGHT   = 10.f;
    const float GAP          = 8.f;
    const float totalW       = ICON_SIZE + GAP + BAR_WIDTH;
    const float startX       = (800.f - totalW) / 2.f;
    const float barX         = startX + ICON_SIZE + GAP;
    const float barY         = HUD_BOTTOM_Y + (ICON_SIZE - BAR_HEIGHT) / 2.f;

    sf::Texture* tex   = nullptr;
    bool loaded        = false;
    float fillRatio    = 1.0f;
    sf::Color barColor = sf::Color::White;

    switch (m_displayedType) {
        case PowerUp::Type::SpeedBoost:
            tex = &m_puIconSpeed; loaded = m_puIconSpeedLoaded;
            fillRatio = (m_speedTimer <= 0.f) ? 0.f : (m_speedTimer / 15.0f);
            barColor  = sf::Color(120, 200, 255); break;
        case PowerUp::Type::SnowballPower:
            tex = &m_puIconSnowball; loaded = m_puIconSnowballLoaded;
            fillRatio = 1.0f; barColor = sf::Color(255, 120, 200); break;
        case PowerUp::Type::DistanceIncrease:
            tex = &m_puIconDistance; loaded = m_puIconDistanceLoaded;
            fillRatio = 1.0f; barColor = sf::Color(80, 140, 255); break;
        case PowerUp::Type::BalloonMode:
            tex = &m_puIconBalloon; loaded = m_puIconBalloonLoaded;
            fillRatio = (m_balloonTimer <= 0.f) ? 0.f : (m_balloonTimer / 10.0f);
            barColor  = sf::Color(180, 220, 255); break;
        default: return;
    }

    if (loaded && tex) {
        sf::Sprite icon(*tex);
        auto ts = tex->getSize();
        if (ts.x > 0 && ts.y > 0)
            icon.setScale({ICON_SIZE / (float)ts.x, ICON_SIZE / (float)ts.y});
        icon.setPosition({startX, HUD_BOTTOM_Y});
        window.draw(icon);
    }

    sf::RectangleShape bg({BAR_WIDTH, BAR_HEIGHT});
    bg.setPosition({barX, barY});
    bg.setFillColor(sf::Color(20, 20, 30, 200));
    bg.setOutlineColor(sf::Color::Black);
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    if (fillRatio > 0.f) {
        sf::RectangleShape fill({BAR_WIDTH * fillRatio, BAR_HEIGHT});
        fill.setPosition({barX, barY});
        fill.setFillColor(barColor);
        window.draw(fill);
    }
}

// ---------------------------------------------------------------
void PlayState::nextLevel() {
    cleanupLevel();
    m_currentLevel++;
    if (m_currentLevel > m_totalLevels) {
        std::cout << "[PlayState] All levels complete!\n";
        return;
    }

    m_levelComplete        = false;
    m_levelTransitionTimer = 0.f;
    m_levelSlideOffset     = 0.f;
    m_showLevelCompleteText = false;
    m_bonusDiamondsSpawned = false;

    // Reload background for new level
    std::string bgPath = "assets/sprites/bg_lvl1.png";
    if (m_currentLevel == 5) bgPath = "assets/sprites/bg_lvl5.png";

    std::cout << "[PlayState] Loading background: " << bgPath << "\n";
    bool bgLoaded = false;
    if (std::FILE* f = std::fopen(bgPath.c_str(), "rb")) {
        std::fclose(f);
        bgLoaded = m_backgroundTexture.loadFromFile(bgPath);
        if (!bgLoaded)
            std::cerr << "[PlayState] fopen OK but loadFromFile FAILED: " << bgPath << "\n";
    } else {
        std::cerr << "[PlayState] FILE NOT FOUND: " << bgPath << "\n";
    }

    if (!bgLoaded) {
        std::cerr << "[PlayState] Falling back to bg_lvl1.png\n";
        bgLoaded = m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png");
    }
    m_backgroundLoaded = bgLoaded;

    if (m_backgroundLoaded) {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_backgroundSprite.setScale({scaleX, scaleY});
    }

    buildLevel();
    g_platforms     = m_platforms;
    g_platformCount = m_platformCount;
    spawnEnemies();

    if (m_player) m_player->respawn(m_playerSpawn);
    std::cout << "[PlayState] Started Level " << m_currentLevel << "\n";
}

// ---------------------------------------------------------------
void PlayState::cleanupLevel() {
    for (int i = 0; i < m_platformCount; ++i) {
        delete m_platforms[i]; m_platforms[i] = nullptr;
    }
    m_platformCount = 0;

    for (int i = 0; i < m_enemyCount; ++i) {
        delete m_enemies[i]; m_enemies[i] = nullptr;
        m_chainCount[i] = 0;
    }
    m_enemyCount = 0;

    for (int i = 0; i < m_projectileCount; ++i) {
        delete m_projectiles[i]; m_projectiles[i] = nullptr;
    }
    m_projectileCount = 0;

    for (int i = 0; i < m_powerUpCount; ++i) {
        delete m_powerUps[i]; m_powerUps[i] = nullptr;
    }
    m_powerUpCount = 0;

    for (int i = 0; i < m_diamondCount; ++i) {
        delete m_diamonds[i]; m_diamonds[i] = nullptr;
    }
    m_diamondCount = 0;

    for (int i = 0; i < m_hitFlashCount; ++i) {
        delete m_hitFlashes[i]; m_hitFlashes[i] = nullptr;
    }
    m_hitFlashCount = 0;

    // Boss cleanup
    delete m_mogera; m_mogera = nullptr;
    for (int i = 0; i < m_mogeraChildCount; ++i) {
        delete m_mogeraChildren[i]; m_mogeraChildren[i] = nullptr;
    }
    m_mogeraChildCount = 0;
}