#include "states/MenuState.hpp"
#include "states/StateManager.hpp"
#include "audio/AudioManager.hpp"
#include "states/PlayState.hpp"
#include "states/LeaderboardState.hpp"
#include <iostream>

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;

    // Button dimensions — half size
    const float BTN_WIDTH  = 130.f;
    const float BTN_HEIGHT = 28.f;
    const unsigned int BTN_TEXT_SIZE = 20;

    // Hover animation
    const float HOVER_SCALE = 1.10f;
    const float NORMAL_SCALE = 1.00f;
    const float SCALE_EASE_SPEED = 12.f;  // higher = snappier

    // Button colors (requested themes)
    const sf::Color COLOR_NAVY   = sf::Color( 25,  45, 120);  // Start Game
    const sf::Color COLOR_GREEN  = sf::Color( 20,  90,  50);  // Leaderboard (bottle green)
    const sf::Color COLOR_RED    = sf::Color(170,  30,  40);  // Exit

    const sf::Color COLOR_OUTLINE = sf::Color(10, 10, 15, 220);
}

// ============================================================================
// Button
// ============================================================================
MenuState::Button::Button(const sf::Font& font)
    : centerRect({BTN_WIDTH, BTN_HEIGHT})
    , leftCap(BTN_HEIGHT / 2.f)
    , rightCap(BTN_HEIGHT / 2.f)
    , shineLeft(1.f)                 // real radius set in configure()
    , shineRight(1.f)
    , shineCenter({1.f, 1.f})
    , outlineRect({1.f, 1.f})
    , outlineLeft(1.f)
    , outlineRight(1.f)
    , text(font, "", BTN_TEXT_SIZE)
    , action(ButtonAction::StartGame)
    , center({0.f, 0.f})
    , baseWidth(BTN_WIDTH)
    , baseHeight(BTN_HEIGHT)
    , currentScale(NORMAL_SCALE)
    , targetScale(NORMAL_SCALE)
    , fillColor(sf::Color::White)
{}

void MenuState::Button::configure(sf::Vector2f centerPos,
                                  float width, float height,
                                  sf::Color baseFill,
                                  const std::string& label)
{
    center = centerPos;
    baseWidth = width;
    baseHeight = height;
    fillColor = baseFill;

    // Fill colors
    centerRect.setFillColor(baseFill);
    leftCap.setFillColor(baseFill);
    rightCap.setFillColor(baseFill);

    // Dark outline (slightly darker version + fully opaque)
    outlineRect.setFillColor(COLOR_OUTLINE);
    outlineLeft.setFillColor(COLOR_OUTLINE);
    outlineRight.setFillColor(COLOR_OUTLINE);

    // Glossy shine — semi-transparent white
    sf::Color shineColor(255, 255, 255, 90);
    shineLeft.setFillColor(shineColor);
    shineRight.setFillColor(shineColor);
    shineCenter.setFillColor(shineColor);

    // Label
    text.setString(label);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color(0, 0, 0, 180));
    text.setOutlineThickness(2.f);

    // Trigger initial layout
    update(0.f);
}

void MenuState::Button::update(float dt) {
    // Ease currentScale toward targetScale. Exponential smoothing:
    // as dt gets smaller, the step gets smaller — frame-rate independent.
    float diff = targetScale - currentScale;
    currentScale += diff * SCALE_EASE_SPEED * dt;
    // Snap if very close, avoids floating-point jitter
    if (std::abs(diff) < 0.001f) currentScale = targetScale;

    // Effective size this frame
    float w = baseWidth  * currentScale;
    float h = baseHeight * currentScale;
    float r = h / 2.f;  // cap radius = half height (perfect capsule)

    // --- Outline (drawn behind, slightly larger) ---
    float outlinePad = 3.f;
    float ow = w + outlinePad * 2.f;
    float oh = h + outlinePad * 2.f;
    float orad = oh / 2.f;

    outlineRect.setSize({ow - oh, oh});
    outlineRect.setPosition({center.x - (ow - oh) / 2.f, center.y - oh / 2.f});

    outlineLeft.setRadius(orad);
    outlineLeft.setPosition({center.x - ow / 2.f, center.y - orad});

    outlineRight.setRadius(orad);
    outlineRight.setPosition({center.x + ow / 2.f - oh, center.y - orad});

    // --- Capsule body ---
    centerRect.setSize({w - h, h});
    centerRect.setPosition({center.x - (w - h) / 2.f, center.y - h / 2.f});

    leftCap.setRadius(r);
    leftCap.setPosition({center.x - w / 2.f, center.y - r});

    rightCap.setRadius(r);
    rightCap.setPosition({center.x + w / 2.f - h, center.y - r});

    // --- Shine (upper half ellipse, fake it with rect + 2 circles) ---
    float shineH = h * 0.45f;
    float shineW = w * 0.82f;
    float shineR = shineH / 2.f;
    float shineTop = center.y - h / 2.f + h * 0.08f;

    shineCenter.setSize({shineW - shineH, shineH});
    shineCenter.setPosition({center.x - (shineW - shineH) / 2.f, shineTop});

    shineLeft.setRadius(shineR);
    shineLeft.setPosition({center.x - shineW / 2.f, shineTop});

    shineRight.setRadius(shineR);
    shineRight.setPosition({center.x + shineW / 2.f - shineH, shineTop});

    // --- Text: also scales with the button ---
    text.setCharacterSize(static_cast<unsigned int>(BTN_TEXT_SIZE * currentScale));
    auto tb = text.getLocalBounds();
    text.setPosition({
        center.x - tb.size.x / 2.f - tb.position.x,
        center.y - tb.size.y / 2.f - tb.position.y
    });
}

void MenuState::Button::draw(sf::RenderWindow& window) const {
    // Draw order: outline -> capsule body -> shine -> text
    window.draw(outlineRect);
    window.draw(outlineLeft);
    window.draw(outlineRight);

    window.draw(centerRect);
    window.draw(leftCap);
    window.draw(rightCap);

    window.draw(shineCenter);
    window.draw(shineLeft);
    window.draw(shineRight);

    window.draw(text);
}

// ============================================================================
// MenuState
// ============================================================================
MenuState::MenuState()
    : m_backgroundSprite(m_backgroundTexture)
    , m_snow(WINDOW_WIDTH, WINDOW_HEIGHT)
    , m_buttonCount(0)
    , m_selectedIndex(0)
{
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        m_buttons[i] = nullptr;
    }
}

MenuState::~MenuState() {
    for (int i = 0; i < m_buttonCount; ++i) {
        delete m_buttons[i];
        m_buttons[i] = nullptr;
    }
}

void MenuState::onEnter() {
    // --- Font ---
    if (!m_bubbleFont.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf")) {
        std::cerr << "[MenuState] Failed to load BubbleBobble-rg3rx.ttf\n";
    }

    // --- Background ---
    if (!m_backgroundTexture.loadFromFile("assets/sprites/menu_bg.png")) {
        std::cerr << "[MenuState] Failed to load menu_bg.png\n";
    } else {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto texSize = m_backgroundTexture.getSize();
        float scaleX = WINDOW_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = WINDOW_HEIGHT / static_cast<float>(texSize.y);
        m_backgroundSprite.setScale({scaleX, scaleY});
        // Light darken so buttons stay readable
        m_backgroundSprite.setColor(sf::Color(200, 200, 200));
    }

    // --- Buttons ---
    // Moved down so the background title art ("SNOW BROS / NICK & TOM / SPECIAL")
    // is fully visible above them.
    float startY = 460.f;
    float spacing = 40.f;
    float centerX = WINDOW_WIDTH / 2.f;

    // Well, the bottom of the background has the monster row at y~1000+,
    // but our window is 600px so we stop at ~580.
    // Actually with startY=420 + 2*70 = 560 — fits within 600 just barely,
    // but we need to check against the monsters at the bottom of the art.
    // If it looks cramped, tweak startY down to ~380.

    addButton("Start Game",  {centerX, startY + 0 * spacing},
              COLOR_NAVY,  ButtonAction::StartGame);
    addButton("Leaderboard", {centerX, startY + 1 * spacing},
              COLOR_GREEN, ButtonAction::Leaderboard);
    addButton("Exit",        {centerX, startY + 2 * spacing},
              COLOR_RED,   ButtonAction::Exit);

    m_selectedIndex = 0;
    setHovered(0);

    // --- Audio ---
    AudioManager::get().playMenuMusic();
}

void MenuState::addButton(const std::string& label, sf::Vector2f center,
                          sf::Color fillColor, ButtonAction action)
{
    if (m_buttonCount >= MAX_BUTTONS) {
        std::cerr << "[MenuState] Button array full\n";
        return;
    }

    Button* btn = new Button(m_bubbleFont);
    btn->action = action;
    btn->configure(center, BTN_WIDTH, BTN_HEIGHT, fillColor, label);

    m_buttons[m_buttonCount] = btn;
    ++m_buttonCount;
}

void MenuState::setHovered(int index) {
    m_selectedIndex = index;
    for (int i = 0; i < m_buttonCount; ++i) {
        m_buttons[i]->targetScale = (i == index) ? HOVER_SCALE : NORMAL_SCALE;
    }
}

int MenuState::buttonAtPoint(sf::Vector2f point) const {
    // Use capsule-approximate hit test: just check the center rect's bounds
    // extended by cap radius. Simpler than true capsule math, plenty accurate.
    for (int i = 0; i < m_buttonCount; ++i) {
        const Button* b = m_buttons[i];
        float w = b->baseWidth;
        float h = b->baseHeight;
        sf::FloatRect hit({b->center.x - w / 2.f, b->center.y - h / 2.f}, {w, h});
        if (hit.contains(point)) {
            return i;
        }
    }
    return -1;
}

void MenuState::activateButton(int index) {
    if (index < 0 || index >= m_buttonCount) return;

    switch (m_buttons[index]->action) {
        case ButtonAction::StartGame:
            std::cout << "[MenuState] Start Game clicked\n";
            m_manager->pushState(new PlayState());
            break;
        case ButtonAction::Leaderboard:
            std::cout << "[MenuState] Leaderboard clicked\n";
            // TODO: push LeaderboardState here
            //Pushing:
            m_manager->pushState(new LeaderboardState());
            break;
        case ButtonAction::Exit:
            std::cout << "[MenuState] Exit clicked\n";
            m_manager->popState();
            break;
    }
}

void MenuState::handleEvent(const sf::Event& event) {
    // Keyboard
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Up) {
            int next = (m_selectedIndex - 1 + m_buttonCount) % m_buttonCount;
            setHovered(next);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Down) {
            int next = (m_selectedIndex + 1) % m_buttonCount;
            setHovered(next);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Enter) {
            activateButton(m_selectedIndex);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Escape) {
            m_manager->popState();
        }
    }

    // Mouse hover
    if (const auto* moveEvent = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(
            static_cast<float>(moveEvent->position.x),
            static_cast<float>(moveEvent->position.y)
        );
        int hovered = buttonAtPoint(mousePos);
        if (hovered != -1 && hovered != m_selectedIndex) {
            setHovered(hovered);
        }
    }

    // Mouse click
    if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (clickEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f clickPos(
                static_cast<float>(clickEvent->position.x),
                static_cast<float>(clickEvent->position.y)
            );
            int clicked = buttonAtPoint(clickPos);
            if (clicked != -1) {
                activateButton(clicked);
            }
        }
    }
}

void MenuState::update(float dt) {
    m_snow.update(dt);
    // Animate buttons toward their target scales
    for (int i = 0; i < m_buttonCount; ++i) {
        m_buttons[i]->update(dt);
    }
}

void MenuState::draw(sf::RenderWindow& window) {
    window.draw(m_backgroundSprite);
    m_snow.draw(window);
    for (int i = 0; i < m_buttonCount; ++i) {
        m_buttons[i]->draw(window);
    }
}