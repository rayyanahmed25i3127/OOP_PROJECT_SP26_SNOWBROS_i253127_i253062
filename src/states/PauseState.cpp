#include "states/PauseState.hpp"
#include "states/StateManager.hpp"
#include <iostream>

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;

    const float BTN_WIDTH  = 220.f;
    const float BTN_HEIGHT = 50.f;
    const unsigned int BTN_TEXT_SIZE = 22;

    const float HOVER_SCALE = 1.10f;
    const float NORMAL_SCALE = 1.00f;
    const float SCALE_EASE_SPEED = 12.f;
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
                                   sf::Color fill, const std::string& label,
                                   Action act)
{
    center = centerPos;
    baseWidth = w;
    baseHeight = h;
    action = act;

    background.setFillColor(fill);
    background.setOutlineThickness(2.f);
    background.setOutlineColor(sf::Color::White);

    text.setString(label);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
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
    : m_title(m_font, "", 48)
    , m_overlay({WINDOW_WIDTH, WINDOW_HEIGHT})
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
    // Reuse the same Bubble Bobble font used in MenuState
    if (!m_font.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf")) {
        std::cerr << "[PauseState] Failed to load font\n";
    }

    // Dark semi-transparent overlay — 65% black
    m_overlay.setFillColor(sf::Color(0, 0, 0, 165));
    m_overlay.setPosition({0.f, 0.f});

    // Title
    m_title.setFont(m_font);
    m_title.setString("PAUSED");
    m_title.setCharacterSize(56);
    m_title.setFillColor(sf::Color(255, 240, 180));
    m_title.setOutlineColor(sf::Color::Black);
    m_title.setOutlineThickness(3.f);
    auto tb = m_title.getLocalBounds();
    m_title.setPosition({
        (WINDOW_WIDTH - tb.size.x) / 2.f - tb.position.x,
        120.f
    });

    // Buttons — centered vertically under the title
    float centerX = WINDOW_WIDTH / 2.f;
    float startY = 260.f;
    float spacing = 70.f;

    m_buttons[0] = new Button(m_font);
    m_buttons[0]->configure({centerX, startY + 0 * spacing},
                            BTN_WIDTH, BTN_HEIGHT,
                            sf::Color( 25,  45, 120), "Resume", Action::Resume);

    m_buttons[1] = new Button(m_font);
    m_buttons[1]->configure({centerX, startY + 1 * spacing},
                            BTN_WIDTH, BTN_HEIGHT,
                            sf::Color( 20,  90,  50), "Main Menu", Action::MainMenu);

    m_buttons[2] = new Button(m_font);
    m_buttons[2]->configure({centerX, startY + 2 * spacing},
                            BTN_WIDTH, BTN_HEIGHT,
                            sf::Color(170,  30,  40), "Exit Game", Action::Exit);

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
    switch (m_buttons[index]->action) {
        case Action::Resume:
            // Just pop self — PlayState resumes underneath
            m_manager->popState();
            break;
        case Action::MainMenu:
            // Pop self AND PlayState — back to MenuState
            m_manager->popState();  // removes PauseState
            m_manager->popState();  // removes PlayState (triggers its onExit)
            break;
        case Action::Exit:
            // Pop everything — empty stack makes Game::run close the window
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
    // PlayState already drew itself underneath us (StateManager handles that
    // because we return true from isTransparent()).
    // Now draw our overlay + UI on top.
    window.draw(m_overlay);
    window.draw(m_title);
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        m_buttons[i]->draw(window);
    }
}