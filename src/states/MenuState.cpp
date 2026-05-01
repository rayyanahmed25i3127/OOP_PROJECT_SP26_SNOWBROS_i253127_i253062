#include "states/MenuState.hpp"
#include "states/StateManager.hpp"
#include "audio/AudioManager.hpp"
#include "states/CharacterSelectState.hpp"   // ← replaces PlayState include
#include "states/LeaderboardState.hpp"
#include <iostream>

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;

    const float BTN_WIDTH  = 130.f;
    const float BTN_HEIGHT = 28.f;
    const unsigned int BTN_TEXT_SIZE = 20;

    const float HOVER_SCALE      = 1.10f;
    const float NORMAL_SCALE     = 1.00f;
    const float SCALE_EASE_SPEED = 12.f;

    const sf::Color COLOR_NAVY    = sf::Color( 25,  45, 120);
    const sf::Color COLOR_SKYBLUE = sf::Color( 70, 160, 220);
    const sf::Color COLOR_GREEN   = sf::Color( 20,  90,  50);
    const sf::Color COLOR_GREY    = sf::Color( 60,  60,  70);
    const sf::Color COLOR_RED     = sf::Color(170,  30,  40);
    const sf::Color COLOR_OUTLINE = sf::Color( 10,  10,  15, 220);
}

// ============================================================================
// Button
// ============================================================================
MenuState::Button::Button(const sf::Font& font)
    : centerRect({BTN_WIDTH, BTN_HEIGHT})
    , leftCap(BTN_HEIGHT / 2.f)
    , rightCap(BTN_HEIGHT / 2.f)
    , shineLeft(1.f)
    , shineRight(1.f)
    , shineCenter({1.f, 1.f})
    , outlineRect({1.f, 1.f})
    , outlineLeft(1.f)
    , outlineRight(1.f)
    , text(font, "", BTN_TEXT_SIZE)
    , action(ButtonAction::NewGame)
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
    center     = centerPos;
    baseWidth  = width;
    baseHeight = height;
    fillColor  = baseFill;

    centerRect.setFillColor(baseFill);
    leftCap.setFillColor(baseFill);
    rightCap.setFillColor(baseFill);

    outlineRect.setFillColor(COLOR_OUTLINE);
    outlineLeft.setFillColor(COLOR_OUTLINE);
    outlineRight.setFillColor(COLOR_OUTLINE);

    sf::Color shineColor(255, 255, 255, 90);
    shineLeft.setFillColor(shineColor);
    shineRight.setFillColor(shineColor);
    shineCenter.setFillColor(shineColor);

    text.setString(label);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color(0, 0, 0, 180));
    text.setOutlineThickness(2.f);

    update(0.f);
}

void MenuState::Button::update(float dt) {
    float diff = targetScale - currentScale;
    currentScale += diff * SCALE_EASE_SPEED * dt;
    if (std::abs(diff) < 0.001f) currentScale = targetScale;

    float w   = baseWidth  * currentScale;
    float h   = baseHeight * currentScale;
    float r   = h / 2.f;

    float outlinePad = 3.f;
    float ow  = w + outlinePad * 2.f;
    float oh  = h + outlinePad * 2.f;
    float orad = oh / 2.f;

    outlineRect.setSize({ow - oh, oh});
    outlineRect.setPosition({center.x - (ow - oh) / 2.f, center.y - oh / 2.f});
    outlineLeft.setRadius(orad);
    outlineLeft.setPosition({center.x - ow / 2.f, center.y - orad});
    outlineRight.setRadius(orad);
    outlineRight.setPosition({center.x + ow / 2.f - oh, center.y - orad});

    centerRect.setSize({w - h, h});
    centerRect.setPosition({center.x - (w - h) / 2.f, center.y - h / 2.f});
    leftCap.setRadius(r);
    leftCap.setPosition({center.x - w / 2.f, center.y - r});
    rightCap.setRadius(r);
    rightCap.setPosition({center.x + w / 2.f - h, center.y - r});

    float shineH   = h * 0.45f;
    float shineW   = w * 0.82f;
    float shineR   = shineH / 2.f;
    float shineTop = center.y - h / 2.f + h * 0.08f;

    shineCenter.setSize({shineW - shineH, shineH});
    shineCenter.setPosition({center.x - (shineW - shineH) / 2.f, shineTop});
    shineLeft.setRadius(shineR);
    shineLeft.setPosition({center.x - shineW / 2.f, shineTop});
    shineRight.setRadius(shineR);
    shineRight.setPosition({center.x + shineW / 2.f - shineH, shineTop});

    text.setCharacterSize(static_cast<unsigned int>(BTN_TEXT_SIZE * currentScale));
    auto tb = text.getLocalBounds();
    text.setPosition({
        center.x - tb.size.x / 2.f - tb.position.x,
        center.y - tb.size.y / 2.f - tb.position.y
    });
}

void MenuState::Button::draw(sf::RenderWindow& window) const {
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
    , m_clickSound(m_clickSoundBuffer)
    , m_clickSoundLoaded(false)
    , m_buttonCount(0)
    , m_selectedIndex(0)
{
    for (int i = 0; i < MAX_BUTTONS; ++i) m_buttons[i] = nullptr;
}

MenuState::~MenuState() {
    for (int i = 0; i < m_buttonCount; ++i) {
        delete m_buttons[i];
        m_buttons[i] = nullptr;
    }
}

void MenuState::onEnter() {
    if (!m_bubbleFont.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf"))
        std::cerr << "[MenuState] Failed to load BubbleBobble-rg3rx.ttf\n";

    if (!m_backgroundTexture.loadFromFile("assets/sprites/menu_bg.png")) {
        std::cerr << "[MenuState] Failed to load menu_bg.png\n";
    } else {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto sz = m_backgroundTexture.getSize();
        m_backgroundSprite.setScale({
            WINDOW_WIDTH  / static_cast<float>(sz.x),
            WINDOW_HEIGHT / static_cast<float>(sz.y)
        });
        m_backgroundSprite.setColor(sf::Color(200, 200, 200));
    }

    if (!m_clickSoundBuffer.loadFromFile("assets/sounds/button_click.wav")) {
        std::cerr << "[MenuState] Failed to load button_click.wav\n";
        m_clickSoundLoaded = false;
    } else {
        m_clickSound.setBuffer(m_clickSoundBuffer);
        m_clickSoundLoaded = true;
    }

    float startY  = 380.f;
    float spacing = 40.f;
    float centerX = WINDOW_WIDTH / 2.f;
    float exitY   = 540.f;

    addButton("New Game",    {centerX, startY + 0 * spacing}, COLOR_NAVY,    ButtonAction::NewGame);
    addButton("Continue",    {centerX, startY + 1 * spacing}, COLOR_SKYBLUE, ButtonAction::Continue);
    addButton("Leaderboard", {centerX, startY + 2 * spacing}, COLOR_GREEN,   ButtonAction::Leaderboard);
    addButton("Logout",      {centerX, startY + 3 * spacing}, COLOR_GREY,    ButtonAction::Logout);
    addButton("Exit",        {centerX, exitY},                COLOR_RED,     ButtonAction::Exit);

    m_selectedIndex = 0;
    setHovered(0);

    AudioManager::get().playMenuMusic();
}

void MenuState::addButton(const std::string& label, sf::Vector2f center,
                          sf::Color fillColor, ButtonAction action)
{
    if (m_buttonCount >= MAX_BUTTONS) {
        std::cerr << "[MenuState] Button array full\n";
        return;
    }
    Button* btn  = new Button(m_bubbleFont);
    btn->action  = action;
    btn->configure(center, BTN_WIDTH, BTN_HEIGHT, fillColor, label);
    m_buttons[m_buttonCount++] = btn;
}

void MenuState::setHovered(int index) {
    m_selectedIndex = index;
    for (int i = 0; i < m_buttonCount; ++i)
        m_buttons[i]->targetScale = (i == index) ? HOVER_SCALE : NORMAL_SCALE;
}

int MenuState::buttonAtPoint(sf::Vector2f point) const {
    for (int i = 0; i < m_buttonCount; ++i) {
        const Button* b = m_buttons[i];
        sf::FloatRect hit(
            {b->center.x - b->baseWidth / 2.f, b->center.y - b->baseHeight / 2.f},
            {b->baseWidth, b->baseHeight}
        );
        if (hit.contains(point)) return i;
    }
    return -1;
}

void MenuState::activateButton(int index) {
    if (index < 0 || index >= m_buttonCount) return;

    if (m_clickSoundLoaded) m_clickSound.play();

    switch (m_buttons[index]->action) {

        case ButtonAction::NewGame:
            std::cout << "[MenuState] New Game → CharacterSelectState\n";
            if (m_clickSoundLoaded) sf::sleep(sf::milliseconds(100));
            // FIX: push CharacterSelectState, NOT PlayState directly.
            // The character select screen will push PlayState once a
            // character is chosen.
            m_manager->pushState(new CharacterSelectState());
            break;

        case ButtonAction::Continue:
            std::cout << "[MenuState] Continue — not yet implemented\n";
            break;

        case ButtonAction::Leaderboard:
            std::cout << "[MenuState] Leaderboard\n";
            m_manager->pushState(new LeaderboardState());
            break;

        case ButtonAction::Logout:
            std::cout << "[MenuState] Logout — not yet implemented\n";
            break;

        case ButtonAction::Exit:
            std::cout << "[MenuState] Exit\n";
            if (m_clickSoundLoaded) sf::sleep(sf::milliseconds(100));
            m_manager->popState();
            break;
    }
}

void MenuState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Up) {
            setHovered((m_selectedIndex - 1 + m_buttonCount) % m_buttonCount);
        } else if (key->code == sf::Keyboard::Key::Down) {
            setHovered((m_selectedIndex + 1) % m_buttonCount);
        } else if (key->code == sf::Keyboard::Key::Enter) {
            activateButton(m_selectedIndex);
        } else if (key->code == sf::Keyboard::Key::Escape) {
            m_manager->popState();
        }
    }

    if (const auto* mv = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f pos(static_cast<float>(mv->position.x),
                         static_cast<float>(mv->position.y));
        int h = buttonAtPoint(pos);
        if (h != -1 && h != m_selectedIndex) setHovered(h);
    }

    if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (click->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos(static_cast<float>(click->position.x),
                             static_cast<float>(click->position.y));
            int h = buttonAtPoint(pos);
            if (h != -1) activateButton(h);
        }
    }
}

void MenuState::update(float dt) {
    m_snow.update(dt);
    for (int i = 0; i < m_buttonCount; ++i) m_buttons[i]->update(dt);
}

void MenuState::draw(sf::RenderWindow& window) {
    window.draw(m_backgroundSprite);
    m_snow.draw(window);
    for (int i = 0; i < m_buttonCount; ++i) m_buttons[i]->draw(window);
}