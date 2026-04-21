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
    : m_backgroundSprite(m_backgroundTexture)  // SFML 3 requires texture on construction
    , m_backgroundLoaded(false)
    , m_player(nullptr)
    , m_platformCount(0)
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
    if (!m_backgroundTexture.loadFromFile("assets/sprites/game_bg.png")) {
        std::cerr << "[PlayState] Could not load game_bg.png — using solid fallback\n";
        m_backgroundLoaded = false;
    } else {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_backgroundSprite.setScale({scaleX, scaleY});
        m_backgroundLoaded = true;
    }

    // --- Spawn player ---
    // Using Anas's Player class directly. Starting position picked so they land
    // on the ground platform.
    m_player = new Player(sf::Vector2f(100.f, 100.f));

    // --- Build level geometry ---
    buildLevel();

    // --- Music switch ---
    AudioManager::get().playGameMusic();
}

void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    // Switch back to menu music when returning to main menu.
    // PauseState doesn't switch music, only leaving PlayState entirely does.
    AudioManager::get().playMenuMusic();
}

void PlayState::buildLevel() {
    // These are the same platforms Anas had in his original main.cpp — we're
    // keeping his layout for continuity. Later a LevelManager will load these
    // from config files per spec section 4.3 (data-driven scalable levels).

    // Ground
    m_platforms[m_platformCount++] =
        new Platform({800.f, 50.f}, {0.f, 550.f});

    // Left raised platform
    m_platforms[m_platformCount++] =
        new Platform({200.f, 20.f}, {100.f, 400.f});

    // Right raised platform
    m_platforms[m_platformCount++] =
        new Platform({150.f, 20.f}, {400.f, 300.f});
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            // Push pause on top — PlayState stays alive underneath
            m_manager->pushState(new PauseState());
        }
        else if (keyEvent->code == sf::Keyboard::Key::F1
              || keyEvent->code == sf::Keyboard::Key::H) {
            // Toggle hitbox debug view (spec 7.2)
            m_showHitboxes = !m_showHitboxes;
            std::cout << "[PlayState] Hitboxes "
                      << (m_showHitboxes ? "ON" : "OFF") << "\n";
        }
    }

    // Note: player input (A/D/Space) is read directly in Player::handleInput()
    // via sf::Keyboard::isKeyPressed, not through events. That's how Anas wrote it.
    // It's fine — realtime key polling vs event-driven are both valid approaches.
}

void PlayState::update(float dt) {
    if (m_player) {
        m_player->update(dt);
    }
    // Platforms are static for now — no update needed.
    // Later: moving platforms would update here.
}

void PlayState::draw(sf::RenderWindow& window) {
    // Draw order: background -> platforms -> player -> debug hitboxes (on top)

    if (m_backgroundLoaded) {
        window.draw(m_backgroundSprite);
    } else {
        // Fallback solid background if image failed
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

    // Debug hitbox overlay (spec colors: green player, blue platform)
    if (m_showHitboxes) {
        if (m_player) {
            m_player->drawHitBoxDebug(window, sf::Color::Green);
        }
        // Platforms don't inherit from Entity so we draw their bounds manually.
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