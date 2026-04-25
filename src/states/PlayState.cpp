#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
#include "states/GameOverState.hpp"
#include "enemies/Botom.hpp"
#include "audio/AudioManager.hpp"
#include "effects/HitFlash.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>   // for date
#include <fstream> // for file handling

// Function to get current date
std::string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);

    char buffer[11];
    sprintf(buffer, "%04d-%02d-%02d",
        1900 + ltm->tm_year,
        1 + ltm->tm_mon,
        ltm->tm_mday);

    return std::string(buffer);
}

// Function to save score to leaderboard
void saveScore(const std::string& name, int score, int level) {
    std::ofstream file("leaderboard.txt", std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Failed to open leaderboard.txt\n";
        return;
    }

    file << name << ","
         << score << " "
         << level << " "
         << getCurrentDate()
         << "\n";

    file.close();
}

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;
}

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
{
    for (int i = 0; i < MAX_HIT_FLASHES; ++i) m_hitFlashes[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES; ++i) m_chainCount[i] = 0;
    for (int i = 0; i < MAX_PROJECTILES; ++i) m_projectiles[i] = nullptr;
    for (int i = 0; i < MAX_PLATFORMS; ++i) m_platforms[i] = nullptr;
    for (int i = 0; i < MAX_ENEMIES;   ++i) {
        m_enemies[i]    = nullptr;
        m_enemyPrevX[i] = 0.f;
        m_enemyPrevY[i] = 0.f;
    }
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
    for (int i = 0; i < m_hitFlashCount; ++i) delete m_hitFlashes[i];
}

void PlayState::onEnter() {
    //mlf
    //nff
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
// --- Spawn player ---
m_player = new Player(m_playerSpawn);

// --- Load HUD assets ---
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

// --- Build level + enemies ---
buildLevel();
spawnEnemies();

    AudioManager::get().playGameMusic();
}

void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    AudioManager::get().playMenuMusic();
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

    // Top-center stepped platform (multi-hitbox)
    {
        const sf::Texture& topTex = m_platformTopTextureLoaded
            ? m_platformTopTexture
            : m_platformTexture;

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
void PlayState::spawnEnemies() {
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(140.f, 400.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(600.f, 400.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(300.f, 290.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(150.f, 180.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(620.f, 180.f));

    std::cout << "[PlayState] Spawned " << m_enemyCount << " Botoms\n";
}
void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (!m_gameOver) {
                m_manager->pushState(new PauseState());
            }
        }
        else if (keyEvent->code == sf::Keyboard::Key::F1
              || keyEvent->code == sf::Keyboard::Key::H) {
            m_showHitboxes = !m_showHitboxes;
            std::cout << "[PlayState] Hitboxes "
                      << (m_showHitboxes ? "ON" : "OFF") << "\n";
        }
    }
}

void PlayState::update(float dt) {
    // --- Player ---
    if (m_player) {
        m_playerPrevX = m_player->getPosition().x;
        m_playerPrevY = m_player->getPosition().y;
        m_player->update(dt);
        m_collider.resolve(*m_player, m_platforms, m_platformCount,
                           m_playerPrevX, m_playerPrevY);
    }

    // --- Enemies ---
    for (int i = 0; i < m_enemyCount; ++i) {
        if (!m_enemies[i]) continue;
        m_enemyPrevX[i] = m_enemies[i]->getPosition().x;
        m_enemyPrevY[i] = m_enemies[i]->getPosition().y;
        m_enemies[i]->update(dt);
        // Rolling enemies have their own motion — don't route through collider.
        if (m_enemies[i]->getState() != Enemy::State::Rolling) {
            m_collider.resolve(*m_enemies[i], m_platforms, m_platformCount,
                               m_enemyPrevX[i], m_enemyPrevY[i]);
        }
    }

    // --- Spawn attack ball ---
    if (m_player && m_player->wantsToThrow()
        && m_projectileCount < MAX_PROJECTILES) {
        sf::Vector2f pPos = m_player->getPosition();
        sf::FloatRect pHit = m_player->getHitBox();
        float spawnY = pHit.position.y + pHit.size.y * 0.3f;
        float spawnX = m_player->isFacingRight()
            ? pHit.position.x + pHit.size.x + 2.f
            : pHit.position.x - 16.f - 2.f;
        m_projectiles[m_projectileCount++] =
            new AttackBall({spawnX, spawnY}, m_player->isFacingRight());
        m_player->consumeThrowRequest();
    }

    // --- Update projectiles ---
    for (int i = 0; i < m_projectileCount; ++i) {
        if (m_projectiles[i]) m_projectiles[i]->update(dt);
    }

    // --- Attack ball vs enemy collision ---
    // Enemy::takeAttackHit() decides whether the hit counts (Alive,
    // PartialEncase, or Escaping75/50/25). PlayState just registers the
    // hit and spawns the flash.
    for (int i = 0; i < m_projectileCount; ++i) {
        if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
        sf::FloatRect pHit = m_projectiles[i]->getHitBox();
        float pL = pHit.position.x, pR = pL + pHit.size.x;
        float pT = pHit.position.y, pB = pT + pHit.size.y;

        for (int e = 0; e < m_enemyCount; ++e) {
            if (!m_enemies[e]) continue;
            Enemy::State s = m_enemies[e]->getState();
            // Skip states that never react to attack balls.
            if (s != Enemy::State::Alive &&
                s != Enemy::State::PartialEncase &&
                s != Enemy::State::Escaping75 &&
                s != Enemy::State::Escaping50 &&
                s != Enemy::State::Escaping25) {
                continue;
            }

            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            float eL = eHit.position.x, eR = eL + eHit.size.x;
            float eT = eHit.position.y, eB = eT + eHit.size.y;
            bool overlap = (pR > eL) && (pL < eR) && (pB > eT) && (pT < eB);
            if (!overlap) continue;

            // Register hit, spawn hit-flash, kill the attack ball.
            m_enemies[e]->takeAttackHit();
            m_projectiles[i]->setAlive(false);

            // Spawn the hit flash at impact point (center of attack ball).
            if (m_hitFlashCount < MAX_HIT_FLASHES) {
                sf::Vector2f flashPos{ (pL + pR) * 0.5f - 6.f,
                                       (pT + pB) * 0.5f - 8.f };
                m_hitFlashes[m_hitFlashCount++] = new HitFlash(flashPos);
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

    // --- PLAYER KICKS SNOWBALLED ENEMY → Rolling ---
    // Auto-kick on contact: if player's hitbox overlaps a Snowballed enemy,
    // launch it rolling in the player's facing direction. Reset its chain
    // count so the first kill scores the base, second adds 10%, etc.
    // --- PLAYER KICKS SNOWBALLED ENEMY → Rolling ---
    // Auto-kick on contact: if player's hitbox overlaps a Snowballed enemy,
    // launch it rolling in the player's facing direction.
    //
    // Spec §9.1: an enemy is "defeated" the moment it's encased and rolled,
    // so we award its base score here at kick-time. Any additional enemies
    // the rolling snowball kills earn base + 10% × chain index on top
    // (handled in the rolling-kill loop below). m_chainCount[e] is set to 1
    // here so the first secondary kill is treated as chain index 2 (+10%),
    // matching "Chain Kill Bonus +10% per enemy in chain".
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

            // Award base score for the enemy that just got rolled.
            int kickedScore = randomScore(100, 500);
            m_score += kickedScore;
            // Chain count = 1 → next victim is chain index 2 → base + 10%.
            m_chainCount[e] = 1;

            std::cout << "[PlayState] Kicked Botom into roll. +"
                      << kickedScore << " (base). Score: "
                      << m_score << "\n";
            break;
        }
    }

    // --- ROLLING ENEMY KILLS (spec §9.1) ---
    // Every enemy a rolling snowball touches awards score immediately.
    //   First kill  = base (100-500 for Botom)
    //   Second kill = base + 10%
    //   Third kill  = base + 20%
    //   etc.
    // Chain counter m_chainCount[r] is reset in kickIntoRoll handling above
    // and carried across GC compaction (see GC block below).
    for (int r = 0; r < m_enemyCount; ++r) {
        if (!m_enemies[r]) continue;
        if (m_enemies[r]->getState() != Enemy::State::Rolling) continue;

        sf::FloatRect rHit = m_enemies[r]->getHitBox();
        float rL = rHit.position.x, rR = rL + rHit.size.x;
        float rT = rHit.position.y, rB = rT + rHit.size.y;

        for (int v = 0; v < m_enemyCount; ++v) {
            if (v == r || !m_enemies[v]) continue;
            if (!m_enemies[v]->isAlive()) continue;   // already killed this frame

            Enemy::State vs = m_enemies[v]->getState();
            // Rolling snowballs pass through each other; dead enemies ignored.
            if (vs == Enemy::State::Dead || vs == Enemy::State::Rolling) continue;

            sf::FloatRect vHit = m_enemies[v]->getHitBox();
            float vL = vHit.position.x, vR = vL + vHit.size.x;
            float vT = vHit.position.y, vB = vT + vHit.size.y;
            bool overlap = (rR > vL) && (rL < vR) && (rB > vT) && (rT < vB);
            if (!overlap) continue;

            // Award score. chainIndex 1 = first kill (no bonus),
            // chainIndex 2 = second kill (+10%), etc.
            m_chainCount[r]++;
            int chainIndex = m_chainCount[r];
            int base  = randomScore(100, 500);
            int bonus = static_cast<int>(base * 0.10f * (chainIndex - 1));
            int award = base + bonus;
            m_score += award;

            std::cout << "[PlayState] Roll kill #" << chainIndex
                      << " +" << award
                      << " (base " << base << " + bonus " << bonus
                      << "). Score: " << m_score << "\n";

            m_enemies[v]->setAlive(false);
        }
    }

    // --- GC dead enemies (compact array) ---
    // Carry chain count along with the enemy pointer when indices shift.
    // Reading m_chainCount[read] then writing to [write] preserves the
    // rolling enemy's kill count across compaction.
    {
        int write = 0;
        for (int read = 0; read < m_enemyCount; ++read) {
            Enemy* e = m_enemies[read];
            bool keep = e && e->isAlive()
                     && e->getState() != Enemy::State::Dead;
            if (keep) {
                int carriedChain = m_chainCount[read];
                m_enemies[write]    = e;
                m_chainCount[write] = carriedChain;
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

    // --- Player-Enemy contact (lethal only on Alive) ---
    if (!m_gameOver && m_player && !m_player->isInvincible()
        && m_collider.checkEnemyContact(*m_player, m_enemies, m_enemyCount)) {
        m_player->loseLife();
        std::cout << "[PlayState] Player lost a life. Lives left: "
                  << m_player->getLives() << "\n";
        if (m_player->getLives() <= 0) {
            m_gameOver = true;
            //mlf:
            // using temperoray now int score = m_score;   // or temporary if not implemented
              int score = rand() % 2000;
            std::string safeName = m_playerName;
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

    // Projectiles drawn above enemies, below player — AttackBalls read cleanly.
    for (int i = 0; i < m_projectileCount; ++i) {
        if (m_projectiles[i]) m_projectiles[i]->draw(window);
    }

    if (m_player) m_player->draw(window);

    // Hit flashes — always drawn (not a debug-only element).
    // Above projectiles/player so impact spark pops on top.
    for (int i = 0; i < m_hitFlashCount; ++i) {
        if (m_hitFlashes[i]) m_hitFlashes[i]->draw(window);
    }

    if (m_showHitboxes) {
        if (m_player) {
            m_player->drawHitBoxDebug(window, sf::Color::Green);
        }
        for (int i = 0; i < m_enemyCount; ++i) {
            if (m_enemies[i]) {
                m_enemies[i]->drawHitBoxDebug(window, sf::Color::Red);
            }
        }
        // Yellow projectile hitboxes per spec §7.2
        for (int i = 0; i < m_projectileCount; ++i) {
            if (m_projectiles[i]) {
                m_projectiles[i]->drawHitBoxDebug(window, sf::Color::Yellow);
            }
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
}

void PlayState::drawHUD(sf::RenderWindow& window) {
    if (!m_hudFontLoaded) return;

    // Anchor HUD just inside the top ice border (over the dark sky area).
    const float HUD_Y       = 14.f;
    const float LEFT_X      = 18.f;
    const float RIGHT_PAD   = 18.f;
    const float ICON_SIZE   = 16.f;
    const float ICON_TEXT_GAP = 6.f;
    const unsigned int TEXT_SIZE = 14;

    // --- Score (top-left, top row) ---
    {
        sf::Text scoreText(m_hudFont, "SCORE " + std::to_string(m_score), TEXT_SIZE);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setOutlineColor(sf::Color::Black);
        scoreText.setOutlineThickness(2.f);
        scoreText.setPosition({ LEFT_X, HUD_Y });
        window.draw(scoreText);
    }

    // --- Lives (top-left, below score) ---
    {
        float livesY = HUD_Y + 22.f;
        int lives = (m_player ? m_player->getLives() : 0);

        if (m_heartLoaded) {
            sf::Sprite heart(m_heartTexture);
            auto ts = m_heartTexture.getSize();
            if (ts.x > 0 && ts.y > 0) {
                heart.setScale({ ICON_SIZE / static_cast<float>(ts.x),
                                 ICON_SIZE / static_cast<float>(ts.y) });
            }
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

    // --- Gems (top-right) ---
    {
        std::string gemStr = std::to_string(m_gems);
        sf::Text gemText(m_hudFont, gemStr, TEXT_SIZE);
        gemText.setFillColor(sf::Color::White);
        gemText.setOutlineColor(sf::Color::Black);
        gemText.setOutlineThickness(2.f);
        auto tb = gemText.getLocalBounds();

        // Right-anchored: text first, icon to its left
        float textX = 800.f - RIGHT_PAD - tb.size.x - tb.position.x;
        gemText.setPosition({ textX, HUD_Y });
        window.draw(gemText);

        if (m_diamondLoaded) {
            sf::Sprite diamond(m_diamondTexture);
            auto ts = m_diamondTexture.getSize();
            if (ts.x > 0 && ts.y > 0) {
                diamond.setScale({ ICON_SIZE / static_cast<float>(ts.x),
                                   ICON_SIZE / static_cast<float>(ts.y) });
            }
            diamond.setPosition({ textX - ICON_SIZE - ICON_TEXT_GAP, HUD_Y });
            window.draw(diamond);
        }
    }

    // --- Level indicator (top-center) ---
    {
        std::string levelStr =
            "LEVEL " + std::to_string(m_currentLevel) +
            "/"      + std::to_string(m_totalLevels);
        sf::Text levelText(m_hudFont, levelStr, TEXT_SIZE);
        levelText.setFillColor(sf::Color::White);
        levelText.setOutlineColor(sf::Color::Black);
        levelText.setOutlineThickness(2.f);
        auto tb = levelText.getLocalBounds();
        levelText.setPosition({
            (800.f - tb.size.x) / 2.f - tb.position.x,
            HUD_Y
        });
        window.draw(levelText);
    }

    // --- Power-up status (bottom-center, placeholder) ---
    {
        sf::Text powText(m_hudFont, "POWER-UP: NONE", 12);
        powText.setFillColor(sf::Color(200, 200, 200));
        powText.setOutlineColor(sf::Color::Black);
        powText.setOutlineThickness(2.f);
        auto tb = powText.getLocalBounds();
        powText.setPosition({
            (800.f - tb.size.x) / 2.f - tb.position.x,
            576.f
        });
        window.draw(powText);
    }
}

int PlayState::randomScore(int lo, int hi) const {
    int range = hi - lo + 1;
    return lo + (std::rand() % range);
}