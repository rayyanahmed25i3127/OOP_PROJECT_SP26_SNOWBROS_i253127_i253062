#include "states/PauseState.hpp"
#include "states/StateManager.hpp"
#include <iostream>
#include <cmath>

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;

    // Capsule button dimensions - iOS style glassy buttons
    const float BTN_WIDTH  = 140.f;  // Reduced width for 2-button rows
    const float BTN_HEIGHT = 45.f;
    const unsigned int BTN_TEXT_SIZE = 25;

    const float HOVER_SCALE = 1.15f;  // 15% zoom on hover
    const float NORMAL_SCALE = 1.00f;
    const float SCALE_EASE_SPEED = 10.f;
}

// ===== Button =====
PauseState::Button::Button(const sf::Font& font)
    : text(font, "", BTN_TEXT_SIZE)
    , background({BTN_WIDTH, BTN_HEIGHT})
    , action(Action::Resume)
    , center({0.f, 0.f})
    , baseWidth(BTN_WIDTH)
    , baseHeight(BTN_HEIGHT)
    , currentScale(NORMAL_SCALE)
    , targetScale(NORMAL_SCALE)
{}

void PauseState::Button::configure(sf::Vector2f centerPos, float w, float h,
                                   const std::string& label, Action act)
{
    center = centerPos;
    baseWidth = w;
    baseHeight = h;
    action = act;

    // iOS-style glassy effect: white with 50% transparency
    background.setFillColor(sf::Color(255, 255, 255, 128));  // 50% transparent white
    background.setOutlineThickness(1.5f);
    background.setOutlineColor(sf::Color(255, 255, 255, 180));  // Slightly more opaque border

    text.setString(label);
    text.setFillColor(sf::Color(255, 255, 255, 255));  // White text
    text.setOutlineColor(sf::Color(0, 0, 0, 150));     // Semi-transparent black outline
    text.setOutlineThickness(2.f);

    update(0.f);
}

void PauseState::Button::update(float dt) {
    float diff = targetScale - currentScale;
    currentScale += diff * SCALE_EASE_SPEED * dt;
    if (std::abs(diff) < 0.001f) currentScale = targetScale;

    float w = baseWidth  * currentScale;
    float h = baseHeight * currentScale;

    background.setSize({w, h});
    background.setPosition({center.x - w / 2.f, center.y - h / 2.f});

    text.setCharacterSize(static_cast<unsigned int>(BTN_TEXT_SIZE * currentScale));
    auto tb = text.getLocalBounds();
    text.setPosition({
        center.x - tb.size.x / 2.f - tb.position.x,
        center.y - tb.size.y / 2.f - tb.position.y
    });
}

void PauseState::Button::draw(sf::RenderWindow& window) const {
    window.draw(background);
    window.draw(text);
}

// ===== PauseState =====
PauseState::PauseState()
    : m_title(m_font, "", 40)
    , m_bgSprite(m_bgTexture)
    , m_bgLoaded(false)
    , m_clickSound(m_clickSoundBuffer)
    , m_clickSoundLoaded(false)
    , m_selectedIndex(0)
{
    for (int i = 0; i < NUM_BUTTONS; ++i) m_buttons[i] = nullptr;
}

PauseState::~PauseState() {
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        delete m_buttons[i];
        m_buttons[i] = nullptr;
    }
}

void PauseState::onEnter() {
    // Load font
    if (!m_font.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf")) {
        std::cerr << "[PauseState] Failed to load font\n";
    }

    // Load background image
    if (!m_bgTexture.loadFromFile("assets/sprites/pause_bg.png")) {
        std::cerr << "[PauseState] Failed to load pause_bg.png\n";
        m_bgLoaded = false;
    } else {
        m_bgSprite.setTexture(m_bgTexture, true);
        auto texSize = m_bgTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_bgSprite.setScale({scaleX, scaleY});
        m_bgLoaded = true;
    }

    // Load button click sound
    if (!m_clickSoundBuffer.loadFromFile("assets/sounds/button_click.wav")) {
        std::cerr << "[PauseState] Failed to load button_click.wav (placeholder - add sound file later)\n";
        m_clickSoundLoaded = false;
    } else {
        m_clickSound.setBuffer(m_clickSoundBuffer);
        m_clickSoundLoaded = true;
    }

    // Title "PAUSED" - positioned below "CLASSIC" text
    m_title.setFont(m_font);
    m_title.setString("PAUSED");
    m_title.setCharacterSize(52);
    m_title.setFillColor(sf::Color(255, 240, 180, 255));
    m_title.setOutlineColor(sf::Color(0, 0, 0, 255));
    m_title.setOutlineThickness(3.f);
    auto tb = m_title.getLocalBounds();
    m_title.setPosition({
        (WINDOW_WIDTH - tb.size.x) / 2.f - tb.position.x,
        310.f  // Position below the logo
    });

    // Button layout:
    // Row 1 (centered): Resume
    // Row 2 (2 buttons): Main Menu | Shop
    // Row 3 (2 buttons): Logout | Exit Game
    
    float centerX = WINDOW_WIDTH / 2.f;
    float resumeY = 390.f;  // Right below "PAUSED" text
    float row2Y = 450.f;    // Second row - reduced spacing
    float row3Y = 505.f;    // Third row - reduced spacing
    float horizontalGap = 80.f;  // Reduced gap between buttons in same row

    // Button 0: Resume (centered, alone in row 1)
    m_buttons[0] = new Button(m_font);
    m_buttons[0]->configure({centerX, resumeY},
                            BTN_WIDTH, BTN_HEIGHT,
                            "Resume", Action::Resume);

    // Button 1: Main Menu (left side of row 2)
    m_buttons[1] = new Button(m_font);
    m_buttons[1]->configure({centerX - horizontalGap, row2Y},
                            BTN_WIDTH, BTN_HEIGHT,
                            "Main Menu", Action::MainMenu);

    // Button 2: Shop (right side of row 2)
    m_buttons[2] = new Button(m_font);
    m_buttons[2]->configure({centerX + horizontalGap, row2Y},
                            BTN_WIDTH, BTN_HEIGHT,
                            "Shop", Action::Shop);

    // Button 3: Logout (left side of row 3)
    m_buttons[3] = new Button(m_font);
    m_buttons[3]->configure({centerX - horizontalGap, row3Y},
                            BTN_WIDTH, BTN_HEIGHT,
                            "Logout", Action::Logout);

    // Button 4: Exit Game (right side of row 3)
    m_buttons[4] = new Button(m_font);
    m_buttons[4]->configure({centerX + horizontalGap, row3Y},
                            BTN_WIDTH, BTN_HEIGHT,
                            "Exit Game", Action::Exit);

    setHovered(0);
}

void PauseState::setHovered(int index) {
    m_selectedIndex = index;
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        m_buttons[i]->targetScale = (i == index) ? HOVER_SCALE : NORMAL_SCALE;
    }
}

int PauseState::buttonAtPoint(sf::Vector2f point) const {
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        const Button* b = m_buttons[i];
        sf::FloatRect hit({b->center.x - b->baseWidth / 2.f,
                           b->center.y - b->baseHeight / 2.f},
                          {b->baseWidth, b->baseHeight});
        if (hit.contains(point)) return i;
    }
    return -1;
}

void PauseState::activateButton(int index) {
    // Play click sound
    if (m_clickSoundLoaded) {
        m_clickSound.play();
    }

    switch (m_buttons[index]->action) {
        case Action::Resume:
            // Small delay to let sound play, then pop self
            if (m_clickSoundLoaded) {
                sf::sleep(sf::milliseconds(100));  // 100ms delay
            }
            m_manager->popState();
            break;
        case Action::MainMenu:
            // Small delay to let sound play
            if (m_clickSoundLoaded) {
                sf::sleep(sf::milliseconds(100));
            }
            m_manager->popState();  // removes PauseState
            m_manager->popState();  // removes PlayState (triggers its onExit)
            break;
        case Action::Shop:
            // TODO: Push ShopState on top of pause menu
            std::cout << "[PauseState] Shop button clicked - not yet implemented\n";
            break;
        case Action::Logout:
            // TODO: Implement logout functionality
            std::cout << "[PauseState] Logout button clicked - not yet implemented\n";
            break;
        case Action::Exit:
            // Small delay to let sound play
            if (m_clickSoundLoaded) {
                sf::sleep(sf::milliseconds(100));
            }
            m_manager->popState();
            m_manager->popState();
            m_manager->popState();  // in case MenuState is also on stack
            break;
    }
}

void PauseState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            // Escape during pause = Resume
            m_manager->popState();
        }
        else if (keyEvent->code == sf::Keyboard::Key::Up) {
            int next = (m_selectedIndex - 1 + NUM_BUTTONS) % NUM_BUTTONS;
            setHovered(next);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Down) {
            int next = (m_selectedIndex + 1) % NUM_BUTTONS;
            setHovered(next);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Enter) {
            activateButton(m_selectedIndex);
        }
    }

    if (const auto* moveEvent = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f pos(static_cast<float>(moveEvent->position.x),
                         static_cast<float>(moveEvent->position.y));
        int h = buttonAtPoint(pos);
        if (h != -1 && h != m_selectedIndex) setHovered(h);
    }

    if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (clickEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos(static_cast<float>(clickEvent->position.x),
                             static_cast<float>(clickEvent->position.y));
            int c = buttonAtPoint(pos);
            if (c != -1) activateButton(c);
        }
    }
}

void PauseState::update(float dt) {
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        m_buttons[i]->update(dt);
    }
}

void PauseState::draw(sf::RenderWindow& window) {
    // Draw background image
    if (m_bgLoaded) {
        window.draw(m_bgSprite);
    } else {
        // Fallback: semi-transparent overlay like before
        sf::RectangleShape fallback({WINDOW_WIDTH, WINDOW_HEIGHT});
        fallback.setFillColor(sf::Color(0, 0, 0, 165));
        window.draw(fallback);
    }

    // Draw title
    window.draw(m_title);

    // Draw buttons
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        m_buttons[i]->draw(window);
    }
}