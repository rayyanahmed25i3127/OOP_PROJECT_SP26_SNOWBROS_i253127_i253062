#include "states/PauseState.hpp"
#include "states/StateManager.hpp"
#include "states/ShopState.hpp"
#include "states/LoginState.hpp"
#include <iostream>
#include <cmath>

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;

    const float BTN_WIDTH   = 140.f;
    const float BTN_HEIGHT  =  45.f;
    const unsigned int BTN_TEXT_SIZE = 25;

    const float HOVER_SCALE     = 1.15f;
    const float NORMAL_SCALE    = 1.00f;
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
    center    = centerPos;
    baseWidth  = w;
    baseHeight = h;
    action    = act;

    // iOS-style glassy: semi-transparent white
    background.setFillColor(sf::Color(255, 255, 255, 128));
    background.setOutlineThickness(1.5f);
    background.setOutlineColor(sf::Color(255, 255, 255, 180));

    text.setString(label);
    text.setFillColor(sf::Color(255, 255, 255, 255));
    text.setOutlineColor(sf::Color(0, 0, 0, 150));
    text.setOutlineThickness(2.f);

    update(0.f);
}

void PauseState::Button::update(float dt)
{
    float diff   = targetScale - currentScale;
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

void PauseState::Button::draw(sf::RenderWindow& window) const
{
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

PauseState::~PauseState()
{
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        delete m_buttons[i];
        m_buttons[i] = nullptr;
    }
}

void PauseState::onEnter()
{
    // ── Font ──────────────────────────────────────────────────────────────────
    if (!m_font.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf"))
        std::cerr << "[PauseState] Failed to load font\n";

    // ── Background ────────────────────────────────────────────────────────────
    if (!m_bgTexture.loadFromFile("assets/sprites/pause_bg.png")) {
        std::cerr << "[PauseState] Failed to load pause_bg.png\n";
        m_bgLoaded = false;
    } else {
        m_bgSprite.setTexture(m_bgTexture, true);
        auto texSize = m_bgTexture.getSize();
        m_bgSprite.setScale({
            WINDOW_WIDTH  / static_cast<float>(texSize.x),
            WINDOW_HEIGHT / static_cast<float>(texSize.y)
        });
        m_bgLoaded = true;
    }

    // ── Click sound ───────────────────────────────────────────────────────────
    if (!m_clickSoundBuffer.loadFromFile("assets/sounds/button_click.wav")) {
        std::cerr << "[PauseState] button_click.wav not found (add later)\n";
        m_clickSoundLoaded = false;
    } else {
        m_clickSound.setBuffer(m_clickSoundBuffer);
        m_clickSoundLoaded = true;
    }

    // ── Title "PAUSED" ────────────────────────────────────────────────────────
    m_title.setFont(m_font);
    m_title.setString("PAUSED");
    m_title.setCharacterSize(52);
    m_title.setFillColor(sf::Color(255, 240, 180, 255));
    m_title.setOutlineColor(sf::Color(0, 0, 0, 255));
    m_title.setOutlineThickness(3.f);

    auto tb = m_title.getLocalBounds();
    m_title.setPosition({
        (WINDOW_WIDTH - tb.size.x) / 2.f - tb.position.x,
        310.f
    });

    // ── Button layout ─────────────────────────────────────────────────────────
    //  Row 1 (centred):         Resume
    //  Row 2 (pair):  Main Menu | Shop
    //  Row 3 (pair):  Logout    | Exit Game
    float centerX       = WINDOW_WIDTH / 2.f;
    float resumeY       = 390.f;
    float row2Y         = 450.f;
    float row3Y         = 505.f;
    float horizontalGap =  80.f;

    m_buttons[0] = new Button(m_font);
    m_buttons[0]->configure({centerX, resumeY},
                             BTN_WIDTH, BTN_HEIGHT,
                             "Resume", Action::Resume);

    m_buttons[1] = new Button(m_font);
    m_buttons[1]->configure({centerX - horizontalGap, row2Y},
                             BTN_WIDTH, BTN_HEIGHT,
                             "Main Menu", Action::MainMenu);

    m_buttons[2] = new Button(m_font);
    m_buttons[2]->configure({centerX + horizontalGap, row2Y},
                             BTN_WIDTH, BTN_HEIGHT,
                             "Shop", Action::Shop);

    m_buttons[3] = new Button(m_font);
    m_buttons[3]->configure({centerX - horizontalGap, row3Y},
                             BTN_WIDTH, BTN_HEIGHT,
                             "Logout", Action::Logout);

    m_buttons[4] = new Button(m_font);
    m_buttons[4]->configure({centerX + horizontalGap, row3Y},
                             BTN_WIDTH, BTN_HEIGHT,
                             "Exit Game", Action::Exit);

    setHovered(0);
}

void PauseState::setHovered(int index)
{
    m_selectedIndex = index;
    for (int i = 0; i < NUM_BUTTONS; ++i)
        m_buttons[i]->targetScale = (i == index) ? HOVER_SCALE : NORMAL_SCALE;
}

int PauseState::buttonAtPoint(sf::Vector2f point) const
{
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        const Button* b = m_buttons[i];
        sf::FloatRect hit({b->center.x - b->baseWidth  / 2.f,
                           b->center.y - b->baseHeight / 2.f},
                          {b->baseWidth, b->baseHeight});
        if (hit.contains(point)) return i;
    }
    return -1;
}

void PauseState::activateButton(int index)
{
    if (m_clickSoundLoaded)
        m_clickSound.play();

    switch (m_buttons[index]->action)
    {
        // ── Resume ────────────────────────────────────────────────────────────
        case Action::Resume:
            if (m_clickSoundLoaded) sf::sleep(sf::milliseconds(100));
            m_manager->popState();
            break;

        // ── Main Menu ─────────────────────────────────────────────────────────
        case Action::MainMenu:
            if (m_clickSoundLoaded) sf::sleep(sf::milliseconds(100));
            m_manager->popState();   // remove PauseState
            m_manager->popState();   // remove PlayState → back to MenuState
            break;

        // ── Shop ──────────────────────────────────────────────────────────────
        //  Push ShopState on top of PauseState.
        //  ShopState::isTransparent() returns false, so it fully covers the screen.
        //  Pressing Back inside ShopState calls popState() → returns here.
        case Action::Shop:
            m_manager->pushState(new ShopState());
            break;

        // ── Logout ────────────────────────────────────────────────────────────
        //  Clear the entire stack and go back to the login screen.
        case Action::Logout:
            if (m_clickSoundLoaded) sf::sleep(sf::milliseconds(100));
            m_manager->replaceState(new LoginState());
            break;

        // ── Exit Game ─────────────────────────────────────────────────────────
        case Action::Exit:
            if (m_clickSoundLoaded) sf::sleep(sf::milliseconds(100));
            // Pop everything — Game::run detects isEmpty() and closes the window
            m_manager->popState();
            m_manager->popState();
            m_manager->popState();
            break;
    }
}

void PauseState::handleEvent(const sf::Event& event)
{
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyEvent->code == sf::Keyboard::Key::Escape)
            m_manager->popState();
        else if (keyEvent->code == sf::Keyboard::Key::Up) {
            setHovered((m_selectedIndex - 1 + NUM_BUTTONS) % NUM_BUTTONS);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Down) {
            setHovered((m_selectedIndex + 1) % NUM_BUTTONS);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Enter)
            activateButton(m_selectedIndex);
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

void PauseState::update(float dt)
{
    for (int i = 0; i < NUM_BUTTONS; ++i)
        m_buttons[i]->update(dt);
}

void PauseState::draw(sf::RenderWindow& window)
{
    if (m_bgLoaded) {
        window.draw(m_bgSprite);
    } else {
        sf::RectangleShape fallback({WINDOW_WIDTH, WINDOW_HEIGHT});
        fallback.setFillColor(sf::Color(0, 0, 0, 165));
        window.draw(fallback);
    }

    window.draw(m_title);

    for (int i = 0; i < NUM_BUTTONS; ++i)
        m_buttons[i]->draw(window);
}