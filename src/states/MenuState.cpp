#include "states/MenuState.hpp"
#include "states/StateManager.hpp"
#include <iostream>

namespace {
    const float BUTTON_WIDTH  = 400.f;
    const float BUTTON_HEIGHT = 60.f;
    const float WINDOW_WIDTH  = 800.f;
    const unsigned int TITLE_SIZE = 48;
    const unsigned int BUTTON_TEXT_SIZE = 22;
}

// --- Button constructor ---
// SFML 3's sf::Text requires a font reference at construction time,
// which is why Button can't be default-constructed — it needs a font.
MenuState::Button::Button(const sf::Font& font)
    : text(font, "", BUTTON_TEXT_SIZE)
    , background({BUTTON_WIDTH, BUTTON_HEIGHT})
    , action(ButtonAction::StartGame)
{}

// --- MenuState constructor ---
MenuState::MenuState()
    : m_title(m_font, "", TITLE_SIZE)
    , m_buttonCount(0)
    , m_selectedIndex(0)
{
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        m_buttons[i] = nullptr;
    }
}

MenuState::~MenuState() {
    // Clean up heap-allocated buttons
    for (int i = 0; i < m_buttonCount; ++i) {
        delete m_buttons[i];
        m_buttons[i] = nullptr;
    }
}

void MenuState::onEnter() {
    // Load the font (SFML 3: openFromFile, not loadFromFile)
    if (!m_font.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        std::cerr << "[MenuState] Failed to load font\n";
    }

    // Title setup
    m_title.setFont(m_font);
    m_title.setString("SNOW BROS");
    m_title.setCharacterSize(TITLE_SIZE);
    m_title.setFillColor(m_colorTitle);

    auto titleBounds = m_title.getLocalBounds();
    m_title.setPosition({
        (WINDOW_WIDTH - titleBounds.size.x) / 2.f,
        80.f
    });

    // Build buttons
    float startY = 260.f;
    float spacing = 80.f;
    addButton("Start Game",  startY + 0 * spacing, ButtonAction::StartGame);
    addButton("Leaderboard", startY + 1 * spacing, ButtonAction::Leaderboard);
    addButton("Exit",        startY + 2 * spacing, ButtonAction::Exit);

    m_selectedIndex = 0;
    refreshButtonVisuals();
}

void MenuState::addButton(const std::string& label, float yPos, ButtonAction action) {
    if (m_buttonCount >= MAX_BUTTONS) {
        std::cerr << "[MenuState] Button array full\n";
        return;
    }

    Button* btn = new Button(m_font);
    btn->text.setString(label);
    btn->action = action;

    btn->background.setPosition({
        (WINDOW_WIDTH - BUTTON_WIDTH) / 2.f,
        yPos
    });
    btn->background.setFillColor(sf::Color(40, 40, 60));
    btn->background.setOutlineThickness(2.f);
    btn->background.setOutlineColor(sf::Color(80, 80, 100));

    // Center text inside the button
    auto textBounds = btn->text.getLocalBounds();
    btn->text.setPosition({
        (WINDOW_WIDTH - textBounds.size.x) / 2.f,
        yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.f - 5.f
    });

    m_buttons[m_buttonCount] = btn;
    ++m_buttonCount;
}

void MenuState::refreshButtonVisuals() {
    for (int i = 0; i < m_buttonCount; ++i) {
        bool selected = (i == m_selectedIndex);
        m_buttons[i]->text.setFillColor(selected ? m_colorSelected : m_colorNormal);
        m_buttons[i]->background.setOutlineColor(
            selected ? m_colorSelected : sf::Color(80, 80, 100)
        );
    }
}

int MenuState::buttonAtPoint(sf::Vector2f point) const {
    for (int i = 0; i < m_buttonCount; ++i) {
        if (m_buttons[i]->background.getGlobalBounds().contains(point)) {
            return i;
        }
    }
    return -1;
}

void MenuState::activateButton(int index) {
    if (index < 0 || index >= m_buttonCount) return;

    // Classic OOP dispatch: switch on the action enum.
    switch (m_buttons[index]->action) {
        case ButtonAction::StartGame:
            std::cout << "[MenuState] Start Game clicked\n";
            // TODO: replace with PlayState when Anas builds Login/you build Play.
            // For now this is a no-op so you can verify the click registers.
            break;

        case ButtonAction::Leaderboard:
            std::cout << "[MenuState] Leaderboard clicked\n";
            // TODO: push LeaderboardState (task #13, Anas).
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
            m_selectedIndex = (m_selectedIndex - 1 + m_buttonCount) % m_buttonCount;
            refreshButtonVisuals();
        }
        else if (keyEvent->code == sf::Keyboard::Key::Down) {
            m_selectedIndex = (m_selectedIndex + 1) % m_buttonCount;
            refreshButtonVisuals();
        }
        else if (keyEvent->code == sf::Keyboard::Key::Enter) {
            activateButton(m_selectedIndex);
        }
        else if (keyEvent->code == sf::Keyboard::Key::Escape) {
            m_manager->popState();
        }
    }

    // Mouse movement (hover)
    if (const auto* moveEvent = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(
            static_cast<float>(moveEvent->position.x),
            static_cast<float>(moveEvent->position.y)
        );
        int hovered = buttonAtPoint(mousePos);
        if (hovered != -1 && hovered != m_selectedIndex) {
            m_selectedIndex = hovered;
            refreshButtonVisuals();
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

void MenuState::update(float /*dt*/) {
    // No per-frame logic yet. Could add animations later.
}

void MenuState::draw(sf::RenderWindow& window) {
    window.draw(m_title);
    for (int i = 0; i < m_buttonCount; ++i) {
        window.draw(m_buttons[i]->background);
        window.draw(m_buttons[i]->text);
    }
}