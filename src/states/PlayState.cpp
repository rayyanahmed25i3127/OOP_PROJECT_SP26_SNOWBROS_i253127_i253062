#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
#include "states/GameOverState.hpp"
#include "enemies/Botom.hpp"
#include "audio/AudioManager.hpp"
#include "effects/HitFlash.hpp"
#include "powerups/PowerUp.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>

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

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;
}

PlayState::PlayState(int characterIndex)
    : m_playerName("")
    , m_characterIndex(characterIndex)
    , m_backgroundTexture()
    , m_backgroundSprite(m_backgroundTexture)
    , m_backgroundLoaded(false)
    , m_projectileCount(0)
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
    , m_puIconSpeed()
    , m_puIconSpeedLoaded(false)
    , m_puIconSnowball()
    , m_puIconSnowballLoaded(false)
    , m_puIconDistance()
    , m_puIconDistanceLoaded(false)
    , m_puIconBalloon()
    , m_puIconBalloonLoaded(false)
    , m_hudFont()
    , m_hudFontLoaded(false)
    , m_heartTexture()
    , m_diamondTexture()
    , m_heartLoaded(false)
    , m_diamondLoaded(false)
    , m_score(0)
    , m_gems(0)
    , m_currentLevel(1)
    , m_totalLevels(10)
    , m_playerSpawn(100.f, 450.f)
    , m_platformTexture()
    , m_platformTopTexture()
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
{
    for (int i = 0; i < MAX_DIAMONDS; ++i) m_diamonds[i] = nullptr;
    for (int i = 0; i < MAX_HIT_FLASHES; ++i) m_hitFlashes[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i) m_chainCount[i] = 0;
    for (int i = 0; i < MAX_PROJECTILES; ++i) m_projectiles[i] = nullptr;
    for (int i = 0; i < MAX_PLATFORMS; ++i) m_platforms[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        m_enemies[i] = nullptr;
        m_enemyPrevX[i] = 0.f;
        m_enemyPrevY[i] = 0.f;
    }
    for (int i = 0; i < MAX_POWERUPS; ++i) m_powerUps[i] = nullptr;
}

PlayState::~PlayState() {
    delete m_player;
    m_player = nullptr;

    for (int i = 0; i < m_platformCount; ++i) {
        delete m_platforms[i];
        m_platforms[i] = nullptr;
    }
    for (int i = 0; i < m_enemyCount; ++i) {
        delete m_enemies[i];
        m_enemies[i] = nullptr;
    }
    for (int i = 0; i < m_projectileCount; ++i) {
        delete m_projectiles[i];
        m_projectiles[i] = nullptr;
    }
    for (int i = 0; i < m_hitFlashCount; ++i) {
        delete m_hitFlashes[i];
        m_hitFlashes[i] = nullptr;
    }
    for (int i = 0; i < m_powerUpCount; ++i) {
        delete m_powerUps[i];
        m_powerUps[i] = nullptr;
    }
    for (int i = 0; i < m_diamondCount; ++i) {
        delete m_diamonds[i];
        m_diamonds[i] = nullptr;
    }
}

void PlayState::onEnter() {
    m_playerName = m_manager->getCurrentUserName();

    std::cout << "[PlayState] Entering gameplay\n";

    if (!m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png")) {
        std::cerr << "[PlayState] Could not load bg_lvl1.png\n";
        m_backgroundLoaded = false;
    } else {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_backgroundSprite.setScale({scaleX, scaleY});
        m_backgroundLoaded = true;
    }

    if (!m_platformTexture.loadFromFile("assets/sprites/platform_1.png")) {
        std::cerr << "[PlayState] Could not load platform_1.png\n";
        m_platformTextureLoaded = false;
    } else {
        m_platformTextureLoaded = true;
    }

    if (!m_platformTopTexture.loadFromFile("assets/sprites/platform_2.png")) {
        std::cerr << "[PlayState] Could not load platform_2.png\n";
        m_platformTopTextureLoaded = false;
    } else {
        m_platformTopTextureLoaded = true;
    }

    m_player = new Player(m_playerSpawn, m_characterIndex);

    // === APPLY SHOP PURCHASES ===
    PlayerProgress& prog = m_manager->getProgress();
    m_gems = prog.gems;
    
    if (prog.pendingSpeed) {
        activatePowerUp(PowerUp::Type::SpeedBoost);
        std::cout << "[PlayState] Applied Speed Boost from shop\n";
    }
    if (prog.pendingSnowball) {
        activatePowerUp(PowerUp::Type::SnowballPower);
        std::cout << "[PlayState] Applied Snowball Power from shop\n";
    }
    if (prog.pendingDistance) {
        activatePowerUp(PowerUp::Type::DistanceIncrease);
        std::cout << "[PlayState] Applied Distance Increase from shop\n";
    }
    if (prog.pendingBalloon) {
        activatePowerUp(PowerUp::Type::BalloonMode);
        std::cout << "[PlayState] Applied Balloon Mode from shop\n";
    }
    if (prog.pendingExtraLifeCount > 0 && m_player) {
        for (int li = 0; li < prog.pendingExtraLifeCount; ++li)
            m_player->addLife();
        std::cout << "[PlayState] Applied " << prog.pendingExtraLifeCount << " Extra Life(s) from shop\n";
    }
    prog.clearPending();

    if (!m_hudFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        std::cerr << "[PlayState] Could not load PressStart2P-Regular.ttf\n";
        m_hudFontLoaded = false;
    } else {
        m_hudFontLoaded = true;
    }

    if (!m_heartTexture.loadFromFile("assets/sprites/heart.png")) {
        std::cerr << "[PlayState] Could not load heart.png\n";
        m_heartLoaded = false;
    } else {
        m_heartLoaded = true;
    }

    if (!m_diamondTexture.loadFromFile("assets/sprites/diamond.png")) {
        std::cerr << "[PlayState] Could not load diamond.png\n";
        m_diamondLoaded = false;
    } else {
        m_diamondLoaded = true;
    }

    loadPowerUpIcons();
    buildLevel();
    spawnEnemies();
    AudioManager::get().playGameMusic();
}

void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    AudioManager::get().playMenuMusic();
}
void PlayState::update(float dt) {
    // === APPLY PENDING SHOP POWERUPS (bought mid-game from pause menu shop) ===
    {
        PlayerProgress& prog = m_manager->getProgress();
        if (prog.pendingSpeed)     { activatePowerUp(PowerUp::Type::SpeedBoost);       prog.pendingSpeed     = false; }
        if (prog.pendingSnowball)  { activatePowerUp(PowerUp::Type::SnowballPower);    prog.pendingSnowball  = false; }
        if (prog.pendingDistance)  { activatePowerUp(PowerUp::Type::DistanceIncrease); prog.pendingDistance  = false; }
        if (prog.pendingBalloon)   { activatePowerUp(PowerUp::Type::BalloonMode);      prog.pendingBalloon   = false; }
        if (prog.pendingExtraLifeCount > 0 && m_player) {
            for (int li = 0; li < prog.pendingExtraLifeCount; ++li)
                m_player->addLife();
            prog.pendingExtraLifeCount = 0;
        }
        // Keep gems in sync (shop may have deducted them)
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
        if (m_enemies[i]->getState() != Enemy::State::Rolling) {
            m_collider.resolve(*m_enemies[i], m_platforms, m_platformCount, m_enemyPrevX[i], m_enemyPrevY[i]);
        }
    }

    if (m_player && m_player->wantsToThrow() && m_projectileCount < MAX_PROJECTILES) {
        sf::Vector2f pPos = m_player->getPosition();
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

    for (int i = 0; i < m_projectileCount; ++i) {
        if (m_projectiles[i]) m_projectiles[i]->update(dt);
    }

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
            float eL = eHit.position.x, eR = eL + eHit.size.x;
            float eT = eHit.position.y, eB = eT + eHit.size.y;
            bool overlap = (pR > eL) && (pL < eR) && (pB > eT) && (pT < eB);
            if (!overlap) continue;

            m_enemies[e]->takeAttackHit();
            m_projectiles[i]->setAlive(false);

            if (m_hitFlashCount < MAX_HIT_FLASHES) {
                sf::Vector2f flashPos{ (pL + pR) * 0.5f - 6.f, (pT + pB) * 0.5f - 8.f };
                m_hitFlashes[m_hitFlashCount++] = new HitFlash(flashPos);
            }
            break;
        }
    }

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

    for (int i = 0; i < m_hitFlashCount; ++i) {
        if (m_hitFlashes[i]) m_hitFlashes[i]->update(dt);
    }
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

    if (m_player) {
        sf::FloatRect pHit = m_player->getHitBox();
        float pL = pHit.position.x, pR = pL + pHit.size.x;
        float pT = pHit.position.y, pB = pT + pHit.size.y;

        for (int e = 0; e < m_enemyCount; ++e) {
            if (!m_enemies[e]) continue;
            if (m_enemies[e]->getState() != Enemy::State::Snowballed) continue;

            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            float eL = eHit.position.x, eR = eL + eHit.size.x;
            float eT = eHit.position.y, eB = eT + eHit.size.y;
            bool overlap = (pR > eL) && (pL < eR) && (pB > eT) && (pT < eB);
            if (!overlap) continue;

            m_enemies[e]->kickIntoRoll(m_player->isFacingRight());
            int kickedScore = randomScore(100, 500);
            m_score += kickedScore;
            m_chainCount[e] = 1;
            std::cout << "[PlayState] Kicked Botom into roll. +" << kickedScore << "\n";
            break;
        }
    }

    // === ROLLING ENEMY KILLS + DIAMOND SPAWN ===
    for (int r = 0; r < m_enemyCount; ++r) {
        if (!m_enemies[r]) continue;
        if (m_enemies[r]->getState() != Enemy::State::Rolling) continue;

        sf::FloatRect rHit = m_enemies[r]->getHitBox();
        float rL = rHit.position.x, rR = rL + rHit.size.x;
        float rT = rHit.position.y, rB = rT + rHit.size.y;

        for (int v = 0; v < m_enemyCount; ++v) {
            if (v == r || !m_enemies[v]) continue;
            if (!m_enemies[v]->isAlive()) continue;

            Enemy::State vs = m_enemies[v]->getState();
            if (vs == Enemy::State::Dead || vs == Enemy::State::Rolling) continue;

            sf::FloatRect vHit = m_enemies[v]->getHitBox();
            float vL = vHit.position.x, vR = vL + vHit.size.x;
            float vT = vHit.position.y, vB = vT + vHit.size.y;
            bool overlap = (rR > vL) && (rL < vR) && (rB > vT) && (rT < vB);
            if (!overlap) continue;

            m_chainCount[r]++;
            int chainIndex = m_chainCount[r];
            int base  = randomScore(100, 500);
            int bonus = static_cast<int>(base * 0.10f * (chainIndex - 1));
            int award = base + bonus;
            m_score += award;

            std::cout << "[PlayState] Roll kill #" << chainIndex << " +" << award << "\n";

            // === SPAWN DIAMOND ON EVERY 2ND CHAIN KILL ===
            if (chainIndex % 2 == 0 && m_diamondCount < MAX_DIAMONDS) {
                sf::Vector2f dropPos = m_enemies[v]->getPosition();
                m_diamonds[m_diamondCount++] = new Diamond(dropPos);
                std::cout << "[PlayState] *** DIAMOND SPAWNED at chain kill #" << chainIndex << " ***\n";
            }

            if (m_powerUpCount < MAX_POWERUPS) {
                int typeIdx = std::rand() % static_cast<int>(PowerUp::Type::Count_);
                PowerUp::Type chosen = static_cast<PowerUp::Type>(typeIdx);
                sf::Vector2f spawnPos = m_enemies[v]->getPosition();
                m_powerUps[m_powerUpCount++] = new PowerUp(spawnPos, chosen);
            }

            m_enemies[v]->setAlive(false);
        }
    }

    // === UPDATE DIAMONDS ===
    for (int i = 0; i < m_diamondCount; ++i) {
        if (!m_diamonds[i]) continue;
        float prevX = m_diamonds[i]->getPosition().x;
        float prevY = m_diamonds[i]->getPosition().y;
        m_diamonds[i]->update(dt);
        m_collider.resolve(*m_diamonds[i], m_platforms, m_platformCount, prevX, prevY);
    }

    // === COLLECT DIAMONDS ===
    if (m_player) {
        sf::FloatRect pHit = m_player->getHitBox();
        for (int i = 0; i < m_diamondCount; ++i) {
            if (!m_diamonds[i] || !m_diamonds[i]->isAlive()) continue;
            sf::FloatRect dHit = m_diamonds[i]->getHitBox();
            bool overlap = (pHit.position.x + pHit.size.x > dHit.position.x) &&
                           (pHit.position.x < dHit.position.x + dHit.size.x) &&
                           (pHit.position.y + pHit.size.y > dHit.position.y) &&
                           (pHit.position.y < dHit.position.y + dHit.size.y);
            if (overlap) {
                m_gems += Diamond::GEM_VALUE;
                m_manager->getProgress().gems = m_gems;
                m_diamonds[i]->setAlive(false);
                std::cout << "[PlayState] *** DIAMOND COLLECTED! +" << Diamond::GEM_VALUE << " gems (Total: " << m_gems << ") ***\n";
            }
        }
    }

    // === GC DEAD DIAMONDS ===
    {
        int write = 0;
        for (int read = 0; read < m_diamondCount; ++read) {
            if (m_diamonds[read] && m_diamonds[read]->isAlive()) {
                m_diamonds[write++] = m_diamonds[read];
            } else {
                delete m_diamonds[read];
                m_diamonds[read] = nullptr;
            }
        }
        m_diamondCount = write;
    }

    for (int i = 0; i < m_powerUpCount; ++i) {
        if (!m_powerUps[i]) continue;
        float prevX = m_powerUps[i]->getPosition().x;
        float prevY = m_powerUps[i]->getPosition().y;
        m_powerUps[i]->update(dt);
        m_collider.resolve(*m_powerUps[i], m_platforms, m_platformCount, prevX, prevY);
    }

    if (m_player) {
        sf::FloatRect pHit = m_player->getHitBox();
        float pL = pHit.position.x, pR = pL + pHit.size.x;
        float pT = pHit.position.y, pB = pT + pHit.size.y;
        for (int i = 0; i < m_powerUpCount; ++i) {
            if (!m_powerUps[i] || !m_powerUps[i]->isAlive()) continue;
            sf::FloatRect h = m_powerUps[i]->getHitBox();
            float hL = h.position.x, hR = hL + h.size.x;
            float hT = h.position.y, hB = hT + h.size.y;
            bool overlap = (pR > hL) && (pL < hR) && (pB > hT) && (pT < hB);
            if (!overlap) continue;

            activatePowerUp(m_powerUps[i]->getType());
            m_powerUps[i]->setAlive(false);
        }
    }

    {
        int write = 0;
        for (int read = 0; read < m_powerUpCount; ++read) {
            if (m_powerUps[read] && m_powerUps[read]->isAlive()) {
                m_powerUps[write++] = m_powerUps[read];
            } else {
                delete m_powerUps[read];
                m_powerUps[read] = nullptr;
            }
        }
        m_powerUpCount = write;
    }

    updatePowerUpTimers(dt);

    {
        int write = 0;
        for (int read = 0; read < m_enemyCount; ++read) {
            Enemy* e = m_enemies[read];
            bool keep = e && e->isAlive() && e->getState() != Enemy::State::Dead;
            if (keep) {
                int carriedChain = m_chainCount[read];
                m_enemies[write] = e;
                m_chainCount[write] = carriedChain;
                ++write;
            } else {
                delete e;
                m_enemies[read] = nullptr;
            }
        }
        for (int i = write; i < m_enemyCount; ++i) {
            m_enemies[i] = nullptr;
            m_chainCount[i] = 0;
        }
        m_enemyCount = write;
    }

    if (!m_levelComplete && m_enemyCount == 0) {
        m_levelComplete = true;
        m_showLevelCompleteText = true;
        m_levelTransitionTimer = 0.f;
        std::cout << "[PlayState] Level " << m_currentLevel << " Complete!\n";
    }

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

    if (!m_gameOver && m_player && !m_player->isInvincible()
        && m_collider.checkEnemyContact(*m_player, m_enemies, m_enemyCount)) {
        m_player->loseLife();
        std::cout << "[PlayState] Player lost a life. Lives left: " << m_player->getLives() << "\n";
        if (m_player->getLives() <= 0) {
            m_gameOver = true;
            int score = m_score;
            std::string currentUser = m_manager->getCurrentUserName();
            saveScore(currentUser, score, m_currentLevel);
            m_manager->pushState(new GameOverState());
        } else {
            m_player->respawn(m_playerSpawn);
        }
    }
}

void PlayState::draw(sf::RenderWindow& window) {
    if (m_backgroundLoaded) {
        window.draw(m_backgroundSprite);
    } else {
        sf::RectangleShape fallback({WINDOW_WIDTH, WINDOW_HEIGHT});
        fallback.setFillColor(sf::Color(20, 30, 50));
        window.draw(fallback);
    }

    for (int i = 0; i < m_platformCount; ++i) {
        m_platforms[i]->draw(window);
    }

    for (int i = 0; i < m_enemyCount; ++i) {
        if (m_enemies[i]) m_enemies[i]->draw(window);
    }

    for (int i = 0; i < m_projectileCount; ++i) {
        if (m_projectiles[i]) m_projectiles[i]->draw(window);
    }

    for (int i = 0; i < m_powerUpCount; ++i) {
        if (m_powerUps[i]) m_powerUps[i]->draw(window);
    }

    for (int i = 0; i < m_diamondCount; ++i) {
        if (m_diamonds[i]) m_diamonds[i]->draw(window);
    }

    if (m_player) m_player->draw(window);

    for (int i = 0; i < m_hitFlashCount; ++i) {
        if (m_hitFlashes[i]) m_hitFlashes[i]->draw(window);
    }

    if (m_showHitboxes) {
        if (m_player) m_player->drawHitBoxDebug(window, sf::Color::Green);
        for (int i = 0; i < m_enemyCount; ++i) {
            if (m_enemies[i]) m_enemies[i]->drawHitBoxDebug(window, sf::Color::Red);
        }
        for (int i = 0; i < m_projectileCount; ++i) {
            if (m_projectiles[i]) m_projectiles[i]->drawHitBoxDebug(window, sf::Color::Yellow);
        }
        for (int i = 0; i < m_powerUpCount; ++i) {
            if (m_powerUps[i]) m_powerUps[i]->drawHitBoxDebug(window, sf::Color::Magenta);
        }
        for (int i = 0; i < m_diamondCount; ++i) {
            if (m_diamonds[i]) m_diamonds[i]->drawHitBoxDebug(window, sf::Color::Cyan);
        }
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
    
    if (m_showLevelCompleteText && m_hudFontLoaded) {
        sf::Text levelCompleteText(m_hudFont);
        std::string message = "Level " + std::to_string(m_currentLevel) + " Complete!";
        levelCompleteText.setString(message);
        levelCompleteText.setCharacterSize(40);
        levelCompleteText.setFillColor(sf::Color::Yellow);
        levelCompleteText.setOutlineColor(sf::Color::Black);
        levelCompleteText.setOutlineThickness(2.f);
        sf::FloatRect bounds = levelCompleteText.getLocalBounds();
        levelCompleteText.setPosition({400.f - bounds.size.x / 2.f, 250.f});
        window.draw(levelCompleteText);
    }
    
    if (m_levelSlideOffset > 0.f) {
        sf::RectangleShape overlay({800.f, 600.f});
        overlay.setFillColor(sf::Color::Black);
        overlay.setPosition({0.f, m_levelSlideOffset - 600.f});
        window.draw(overlay);
    }
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (!m_gameOver) m_manager->pushState(new PauseState());
        }
        else if (keyEvent->code == sf::Keyboard::Key::F1 || keyEvent->code == sf::Keyboard::Key::H) {
            m_showHitboxes = !m_showHitboxes;
            std::cout << "[PlayState] Hitboxes " << (m_showHitboxes ? "ON" : "OFF") << "\n";
        }
    }
}

// HELPER METHODS - Add all remaining methods here...
void PlayState::buildLevel() {
    if (!m_platformTextureLoaded) return;
    const float BORDER_W = 8.5f, LEFT_EXTRA = 22.f;
    const float p1H = 78.75f, p2H = 150.f, p2W = 630.f;

    {
        const sf::Texture& topTex = m_platformTopTextureLoaded ? m_platformTopTexture : m_platformTexture;
        sf::Vector2f p2Size = { p2W, p2H };
        sf::Vector2f p2Pos  = { (WINDOW_WIDTH - p2W) / 2.f, 49.f };
        sf::FloatRect lower; lower.position = { 30.f, 88.f }; lower.size = { p2W - 60.f, 35.f };
        sf::FloatRect upper; upper.position = { 130.f, 45.5f }; upper.size = { p2W - 265.f, 35.f };
        sf::FloatRect boxes[2] = { lower, upper };
        bool solids[2] = { false, true };
        m_platforms[m_platformCount++] = new Platform(topTex, p2Size, p2Pos, boxes, 2, solids);
    }

    float w = 300.f;
    m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 220.f});
    m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 220.f});
    
    w = 390.f;
    m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {(WINDOW_WIDTH - w) / 2.f, 329.f});
    
    w = 300.f;
    m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 437.f});
    m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 437.f});
}

void PlayState::spawnEnemies() {
    if (m_currentLevel == 1) {
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(140.f, 400.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(600.f, 400.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(300.f, 290.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(150.f, 180.f));
        m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(620.f, 180.f));
    } else {
        std::srand(static_cast<unsigned>(std::time(nullptr)) + m_currentLevel);
        float positions[] = {140.f, 300.f, 450.f, 600.f, 150.f, 350.f, 500.f, 650.f};
        float yLevels[] = {400.f, 290.f, 180.f};
        for (int i = 0; i < 8; ++i) {
            float x = positions[i];
            float y = yLevels[std::rand() % 3];
            m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(x, y));
        }
    }

    if (m_snowballPowerActive) {
        for (int i = 0; i < m_enemyCount; ++i) {
            if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true);
        }
    }
}

void PlayState::nextLevel() {
    cleanupLevel();
    m_currentLevel++;
    if (m_currentLevel > m_totalLevels) return;
    m_levelComplete = false;
    m_levelTransitionTimer = 0.f;
    m_levelSlideOffset = 0.f;
    m_showLevelCompleteText = false;
    buildLevel();
    spawnEnemies();
    if (m_player) m_player->respawn(m_playerSpawn);
}

void PlayState::cleanupLevel() {
    for (int i = 0; i < m_platformCount; ++i) {
        delete m_platforms[i];
        m_platforms[i] = nullptr;
    }
    m_platformCount = 0;
    
    for (int i = 0; i < m_enemyCount; ++i) {
        delete m_enemies[i];
        m_enemies[i] = nullptr;
        m_chainCount[i] = 0;
    }
    m_enemyCount = 0;
    
    for (int i = 0; i < m_projectileCount; ++i) {
        delete m_projectiles[i];
        m_projectiles[i] = nullptr;
    }
    m_projectileCount = 0;
    
    for (int i = 0; i < m_powerUpCount; ++i) {
        delete m_powerUps[i];
        m_powerUps[i] = nullptr;
    }
    m_powerUpCount = 0;
    
    for (int i = 0; i < m_diamondCount; ++i) {
        delete m_diamonds[i];
        m_diamonds[i] = nullptr;
    }
    m_diamondCount = 0;
    
    for (int i = 0; i < m_hitFlashCount; ++i) {
        delete m_hitFlashes[i];
        m_hitFlashes[i] = nullptr;
    }
    m_hitFlashCount = 0;
}

int PlayState::randomScore(int lo, int hi) const {
    int range = hi - lo + 1;
    return lo + (std::rand() % range);
}

void PlayState::drawHUD(sf::RenderWindow& window) {
    if (!m_hudFontLoaded) return;
    const float HUD_Y = 14.f, LEFT_X = 18.f, RIGHT_PAD = 18.f;
    const float ICON_SIZE = 16.f, ICON_TEXT_GAP = 6.f;
    const unsigned int TEXT_SIZE = 14;

    {
        sf::Text scoreText(m_hudFont, "SCORE " + std::to_string(m_score), TEXT_SIZE);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setOutlineColor(sf::Color::Black);
        scoreText.setOutlineThickness(2.f);
        scoreText.setPosition({ LEFT_X, HUD_Y });
        window.draw(scoreText);
    }

    {
        float livesY = HUD_Y + 22.f;
        int lives = (m_player ? m_player->getLives() : 0);
        if (m_heartLoaded) {
            sf::Sprite heart(m_heartTexture);
            auto ts = m_heartTexture.getSize();
            if (ts.x > 0 && ts.y > 0) heart.setScale({ ICON_SIZE / static_cast<float>(ts.x), ICON_SIZE / static_cast<float>(ts.y) });
            heart.setPosition({ LEFT_X, livesY });
            window.draw(heart);
        }
        sf::Text livesText(m_hudFont, std::to_string(lives), TEXT_SIZE);
        livesText.setFillColor(sf::Color::White);
        livesText.setOutlineColor(sf::Color::Black);
        livesText.setOutlineThickness(2.f);
        livesText.setPosition({ LEFT_X + ICON_SIZE + ICON_TEXT_GAP, livesY });
        window.draw(livesText);
    }

    {
        std::string gemStr = std::to_string(m_gems);
        sf::Text gemText(m_hudFont, gemStr, TEXT_SIZE);
        gemText.setFillColor(sf::Color::White);
        gemText.setOutlineColor(sf::Color::Black);
        gemText.setOutlineThickness(2.f);
        auto tb = gemText.getLocalBounds();
        float textX = 800.f - RIGHT_PAD - tb.size.x - tb.position.x;
        gemText.setPosition({ textX, HUD_Y });
        window.draw(gemText);

        if (m_diamondLoaded) {
            sf::Sprite diamond(m_diamondTexture);
            auto ts = m_diamondTexture.getSize();
            if (ts.x > 0 && ts.y > 0) diamond.setScale({ ICON_SIZE / static_cast<float>(ts.x), ICON_SIZE / static_cast<float>(ts.y) });
            diamond.setPosition({ textX - ICON_SIZE - ICON_TEXT_GAP, HUD_Y });
            window.draw(diamond);
        }
    }

    {
        std::string levelStr = "LEVEL " + std::to_string(m_currentLevel) + "/" + std::to_string(m_totalLevels);
        sf::Text levelText(m_hudFont, levelStr, TEXT_SIZE);
        levelText.setFillColor(sf::Color::White);
        levelText.setOutlineColor(sf::Color::Black);
        levelText.setOutlineThickness(2.f);
        auto tb = levelText.getLocalBounds();
        levelText.setPosition({ (800.f - tb.size.x) / 2.f - tb.position.x, HUD_Y });
        window.draw(levelText);
    }

    drawPowerUpHUD(window);
}

void PlayState::loadPowerUpIcons() {
    auto tryLoad = [](sf::Texture& tex, const char* path, bool& flag) {
        if (std::FILE* f = std::fopen(path, "rb")) {
            std::fclose(f);
            flag = tex.loadFromFile(path);
        } else {
            flag = false;
        }
    };
    tryLoad(m_puIconSpeed, "assets/sprites/powerup_speed.png", m_puIconSpeedLoaded);
    tryLoad(m_puIconSnowball, "assets/sprites/powerup_snowball.png", m_puIconSnowballLoaded);
    tryLoad(m_puIconDistance, "assets/sprites/powerup_distance.png", m_puIconDistanceLoaded);
    tryLoad(m_puIconBalloon, "assets/sprites/powerup_balloon.png", m_puIconBalloonLoaded);
}

void PlayState::activatePowerUp(PowerUp::Type type) {
    switch (type) {
        case PowerUp::Type::SpeedBoost:
            m_speedActive = true;
            m_speedTimer = 15.0f;
            if (m_player) m_player->setSpeedMultiplier(1.5f);
            break;
        case PowerUp::Type::SnowballPower:
            m_snowballPowerActive = true;
            for (int i = 0; i < m_enemyCount; ++i) {
                if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true);
            }
            break;
        case PowerUp::Type::DistanceIncrease:
            m_distanceActive = true;
            break;
        case PowerUp::Type::BalloonMode:
            m_balloonActive = true;
            m_balloonTimer = 10.0f;
            if (m_player) m_player->setBalloonMode(true);
            break;
        default: return;
    }
    m_displayedType = type;
    m_hasDisplayed = true;
    std::cout << "[PlayState] Activated power-up: " << PowerUp::typeName(type) << "\n";
}

void PlayState::updatePowerUpTimers(float dt) {
    if (m_speedActive) {
        m_speedTimer -= dt;
        if (m_speedTimer <= 0.f) {
            m_speedActive = false;
            m_speedTimer = 0.f;
            if (m_player) m_player->setSpeedMultiplier(1.0f);
            if (m_hasDisplayed && m_displayedType == PowerUp::Type::SpeedBoost) {
                if (m_balloonActive) m_displayedType = PowerUp::Type::BalloonMode;
                else if (m_snowballPowerActive) m_displayedType = PowerUp::Type::SnowballPower;
                else if (m_distanceActive) m_displayedType = PowerUp::Type::DistanceIncrease;
                else m_hasDisplayed = false;
            }
        }
    }
    if (m_balloonActive) {
        m_balloonTimer -= dt;
        if (m_balloonTimer <= 0.f) {
            m_balloonActive = false;
            m_balloonTimer = 0.f;
            if (m_player) m_player->setBalloonMode(false);
            if (m_hasDisplayed && m_displayedType == PowerUp::Type::BalloonMode) {
                if (m_speedActive) m_displayedType = PowerUp::Type::SpeedBoost;
                else if (m_snowballPowerActive) m_displayedType = PowerUp::Type::SnowballPower;
                else if (m_distanceActive) m_displayedType = PowerUp::Type::DistanceIncrease;
                else m_hasDisplayed = false;
            }
        }
    }
}

void PlayState::drawPowerUpHUD(sf::RenderWindow& window) {
    if (!m_hasDisplayed) return;
    const float HUD_BOTTOM_Y = 568.f, ICON_SIZE = 22.f;
    const float BAR_WIDTH = 160.f, BAR_HEIGHT = 10.f, GAP = 8.f;
    const float totalW = ICON_SIZE + GAP + BAR_WIDTH;
    const float startX = (800.f - totalW) / 2.f;
    const float iconX = startX, barX = startX + ICON_SIZE + GAP;
    const float iconY = HUD_BOTTOM_Y, barY = HUD_BOTTOM_Y + (ICON_SIZE - BAR_HEIGHT) / 2.f;

    sf::Texture* tex = nullptr;
    bool loaded = false;
    float fillRatio = 1.0f;
    sf::Color barColor = sf::Color::White;

    switch (m_displayedType) {
        case PowerUp::Type::SpeedBoost:
            tex = &m_puIconSpeed; loaded = m_puIconSpeedLoaded;
            fillRatio = (m_speedTimer <= 0.f) ? 0.f : (m_speedTimer / 15.0f);
            barColor = sf::Color(120, 200, 255);
            break;
        case PowerUp::Type::SnowballPower:
            tex = &m_puIconSnowball; loaded = m_puIconSnowballLoaded;
            fillRatio = 1.0f;
            barColor = sf::Color(255, 120, 200);
            break;
        case PowerUp::Type::DistanceIncrease:
            tex = &m_puIconDistance; loaded = m_puIconDistanceLoaded;
            fillRatio = 1.0f;
            barColor = sf::Color(80, 140, 255);
            break;
        case PowerUp::Type::BalloonMode:
            tex = &m_puIconBalloon; loaded = m_puIconBalloonLoaded;
            fillRatio = (m_balloonTimer <= 0.f) ? 0.f : (m_balloonTimer / 10.0f);
            barColor = sf::Color(180, 220, 255);
            break;
        default: return;
    }

    if (loaded && tex) {
        sf::Sprite icon(*tex);
        auto ts = tex->getSize();
        if (ts.x > 0 && ts.y > 0) icon.setScale({ ICON_SIZE / static_cast<float>(ts.x), ICON_SIZE / static_cast<float>(ts.y) });
        icon.setPosition({ iconX, iconY });
        window.draw(icon);
    }

    sf::RectangleShape bg({ BAR_WIDTH, BAR_HEIGHT });
    bg.setPosition({ barX, barY });
    bg.setFillColor(sf::Color(20, 20, 30, 200));
    bg.setOutlineColor(sf::Color::Black);
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    if (fillRatio > 0.f) {
        sf::RectangleShape fill({ BAR_WIDTH * fillRatio, BAR_HEIGHT });
        fill.setPosition({ barX, barY });
        fill.setFillColor(barColor);
        window.draw(fill);
    }
}