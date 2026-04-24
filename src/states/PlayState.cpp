#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
#include "states/GameOverState.hpp"
#include "enemies/Botom.hpp"
#include "audio/AudioManager.hpp"
#include <iostream>

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
{
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
}

void PlayState::onEnter() {
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

        m_collider.resolve(*m_enemies[i], m_platforms, m_platformCount,
                           m_enemyPrevX[i], m_enemyPrevY[i]);
    }

    // --- Player-Enemy contact check ---
    // Routed through CollisionDetector per spec §7.2 ("no ad-hoc overlap checks").
    // m_gameOver guard prevents pushing multiple GameOverStates if contact
    // persists during the one-frame gap before GameOverState activates.
    // --- Player-Enemy contact -> lose a life (or game over if out of lives) ---
    // Spec §7.3: touching an enemy costs 1 life. Player gets a brief
    // invincibility window after losing a life so a single prolonged
    // overlap doesn't drain every life in one frame.
    if (!m_gameOver && m_player && !m_player->isInvincible()
        && m_collider.checkEnemyContact(*m_player, m_enemies, m_enemyCount))
    {
        m_player->loseLife();
        std::cout << "[PlayState] Player lost a life. Lives left: "
                  << m_player->getLives() << "\n";

        if (m_player->getLives() <= 0) {
            m_gameOver = true;
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

    if (m_player) m_player->draw(window);

    if (m_showHitboxes) {
        if (m_player) {
            m_player->drawHitBoxDebug(window, sf::Color::Green);
        }

        for (int i = 0; i < m_enemyCount; ++i) {
            if (m_enemies[i]) {
                m_enemies[i]->drawHitBoxDebug(window, sf::Color::Red);
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