#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
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
    , m_collider(30.f, 770.f)   // ice border walls
    , m_playerPrevY(0.f)
    , m_showHitboxes(false)
{
    for (int i = 0; i < MAX_PLATFORMS; ++i) {
        m_platforms[i] = nullptr;
    }
}

PlayState::~PlayState() {
    delete m_player;
    m_player = nullptr;

    for (int i = 0; i < m_platformCount; ++i) {
        delete m_platforms[i];
        m_platforms[i] = nullptr;
    }
}

void PlayState::onEnter() {
    std::cout << "[PlayState] Entering gameplay\n";

    // --- Load background ---
    if (!m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png")) {
        std::cerr << "[PlayState] Could not load bg_lvl1.png — using solid fallback\n";
        m_backgroundLoaded = false;
    } else {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_backgroundSprite.setScale({scaleX, scaleY});
        m_backgroundLoaded = true;
    }

    // --- Load standard platform tile ---
    if (!m_platformTexture.loadFromFile("assets/sprites/platform_1.png")) {
        std::cerr << "[PlayState] Could not load platform_1.png\n";
        m_platformTextureLoaded = false;
    } else {
        m_platformTextureLoaded = true;
    }

    // --- Load decorative top platform tile ---
    if (!m_platformTopTexture.loadFromFile("assets/sprites/platform_2.png")) {
        std::cerr << "[PlayState] Could not load platform_2.png\n";
        m_platformTopTextureLoaded = false;
    } else {
        m_platformTopTextureLoaded = true;
    }
    // --- Spawn player ---
    // Spawn on the ground, left-ish side. Y is chosen so the hitbox
    // bottom will naturally settle at the snow line on first frame.
    m_player = new Player(sf::Vector2f(100.f, 450.f));

    // --- Build level geometry ---
   buildLevel();

    // --- Music switch ---
    AudioManager::get().playGameMusic();
}

void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    AudioManager::get().playMenuMusic();
}

void PlayState::buildLevel() {
    // Snow Bros level 1.
    // NOTE: No ground platform is drawn — the bg_lvl1.png's bottom ice border
    // acts as the visual ground.
    // NOTE: bg_lvl1.png's left border is slightly sloped/angled, so left-side
    // platforms need a small extra inset to avoid overlapping the ice cubes.
    // Right border is clean vertical, so right-side platforms sit at BORDER_W.
    if (!m_platformTextureLoaded) {
        std::cerr << "[PlayState] Skipping platform build — texture missing\n";
        return;
    }
    // Adjustable constants
    const float BORDER_W   = 8.5f;
    const float LEFT_EXTRA = 22.f;   // extra inset for left platforms to clear the slope

    // Platform_1 (standard ice tile) sizing:
    //   height: 75 * 1.05 = 78.75
    //   widths bumped up modestly as well
    const float p1H = 78.75f;
    // Platform_2 (decorative top) sizing:
    //   height: 75 * 2.0 = 150
    //   width:  420 * 1.5 = 630
    const float p2H = 150.f;
    const float p2W = 630.f;
    // --- Top-center (platform_2.png — decorative) ---
    const sf::Texture& topTex = m_platformTopTextureLoaded
        ? m_platformTopTexture
        : m_platformTexture;
    m_platforms[m_platformCount++] =
        new Platform(topTex, {p2W, p2H}, {(WINDOW_WIDTH - p2W) / 2.f, 49.f});
    // --- Upper-left & upper-right (flush against borders) ---
    // Width bumped: 260 * ~1.15 ≈ 300
    {
        float w = 300.f;
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 220.f});
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 220.f});
    }
    // --- Mid-center ---
    // Width bumped: 340 * ~1.15 ≈ 390
    {
        float w = 390.f;
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {(WINDOW_WIDTH - w) / 2.f, 329.f});
    }
    // --- Lower-left & lower-right (flush against borders) ---
    {
        float w = 300.f;
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 437.f});
        m_platforms[m_platformCount++] =
            new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 437.f});
    }
}
//ANAS ADDING FOR LEVEL 2

//added till here

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            m_manager->pushState(new PauseState());
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
    if (m_player) {
        // Capture Y position BEFORE the player moves — collision
        // needs this to know if the player was above a platform last frame.
        m_playerPrevY = m_player->getPosition().y;

        // Player moves (applies velocity, gravity)
        m_player->update(dt);

        // Resolve collisions against walls and platforms
        m_collider.resolve(*m_player, m_platforms, m_platformCount, m_playerPrevY);
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

    if (m_player) {
        m_player->draw(window);
    }

    if (m_showHitboxes) {
        if (m_player) {
            m_player->drawHitBoxDebug(window, sf::Color::Green);
        }
        for (int i = 0; i < m_platformCount; ++i) {
            sf::FloatRect b = m_platforms[i]->getBounds();
            sf::RectangleShape box({b.size.x, b.size.y});
            box.setPosition({b.position.x, b.position.y});
            box.setFillColor(sf::Color::Transparent);
            box.setOutlineColor(sf::Color::Blue);
            box.setOutlineThickness(1.f);
            window.draw(box);
        }
    }
}