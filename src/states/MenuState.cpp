#include "states/MenuState.hpp"
#include "states/StateManager.hpp"
#include <iostream>

// Constants for layout — kept local to this file (not in header) because
// nothing else needs them. Making them named constants makes the code
// self-documenting and easy to tweak.
namespace {
    constexpr float BUTTON_WIDTH  = 400.f;
    constexpr float BUTTON_HEIGHT = 60.f;
    constexpr float WINDOW_WIDTH  = 800.f;
    constexpr float WINDOW_HEIGHT = 600.f;
    constexpr unsigned int TITLE_SIZE  = 48;
    constexpr unsigned int BUTTON_TEXT_SIZE = 22;
}

MenuState::MenuState()
    // SFML 3 requires sf::Text to be constructed with a font reference.
    // We can't construct m_title properly here because m_font hasn't loaded yet.
    // So we use a placeholder and fully initialize in onEnter().
    : m_title(m_font, "", TITLE_SIZE)
{
    // Intentionally empty — all setup happens in onEnter().
    // Why? Because if font loading fails, we want to fail AFTER the state is
    // on the stack (so we can log it), not during construction.
}

void MenuState::onEnter() {
    // Load the font from disk. In SFML 3 this is openFromFile (was loadFromFile in SFML 2).
    // Path is relative to the executable's working directory (build/ folder),
    // but because our CMakeLists copies assets/ into build/, this just works.
    if (!m_font.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        std::cerr << "[MenuState] Failed to load font: assets/fonts/PressStart2P-Regular.ttf\n";
        // We don't crash — the game will still run, just with ugly default text.
    }

    // Configure the title
    m_title.setFont(m_font);
    m_title.setString("SNOW BROS");
    m_title.setCharacterSize(TITLE_SIZE);
    m_title.setFillColor(m_colorTitle);

    // Center the title horizontally. getLocalBounds() returns the text's bounding box;
    // we use its width to compute the centered x-position.
    auto titleBounds = m_title.getLocalBounds();
    m_title.setPosition({
        (WINDOW_WIDTH - titleBounds.size.x) / 2.f,
        80.f  // fixed distance from top
    });

    // Build the three buttons. Lambdas capture `this` so they can call m_manager.
    // Buttons are laid out vertically, evenly spaced.
    float startY = 260.f;
    float spacing = 80.f;

    addButton("Start Game", startY + 0 * spacing, [this]() {
        std::cout << "[MenuState] Start Game clicked\n";
        // TODO: replace with PlayState when it exists.
        // For now this is a no-op so you can see the click registers in the console.
    });

    addButton("Leaderboard", startY + 1 * spacing, [this]() {
        std::cout << "[MenuState] Leaderboard clicked\n";
        // TODO: push LeaderboardState (Anas's task #13).
    });

    addButton("Exit", startY + 2 * spacing, [this]() {
        std::cout << "[MenuState] Exit clicked\n";
        // Pop this state. When the stack becomes empty, Game::run() closes the window.
        m_manager->popState();
    });

    // Highlight the first button by default (for keyboard users)
    m_selectedIndex = 0;
    refreshButtonVisuals();
}

void MenuState::addButton(const std::string& label, float yPos, std::function<void()> onActivate) {
    Button btn{
        sf::Text(m_font, label, BUTTON_TEXT_SIZE),  // text
        sf::RectangleShape({BUTTON_WIDTH, BUTTON_HEIGHT}), // background
        std::move(onActivate)                        // callback
    };

    // Position the background rectangle (centered horizontally)
    btn.background.setPosition({
        (WINDOW_WIDTH - BUTTON_WIDTH) / 2.f,
        yPos
    });
    btn.background.setFillColor(sf::Color(40, 40, 60));      // dark slate fill
    btn.background.setOutlineThickness(2.f);
    btn.background.setOutlineColor(sf::Color(80, 80, 100));  // subtle border

    // Center the label inside the button rectangle.
    auto textBounds = btn.text.getLocalBounds();
    btn.text.setPosition({
        (WINDOW_WIDTH - textBounds.size.x) / 2.f,
        yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.f - 5.f  // -5 for optical centering
    });

    m_buttons.push_back(std::move(btn));
}

void MenuState::refreshButtonVisuals() {
    // Walk all buttons and color them based on whether they're the selected one.
    for (std::size_t i = 0; i < m_buttons.size(); ++i) {
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        m_buttons[i].text.setFillColor(selected ? m_colorSelected : m_colorNormal);
        m_buttons[i].background.setOutlineColor(
            selected ? m_colorSelected : sf::Color(80, 80, 100)
        );
    }
}

int MenuState::buttonAtPoint(sf::Vector2f point) const {
    for (std::size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].background.getGlobalBounds().contains(point)) {
            return static_cast<int>(i);
        }
    }
    return -1;  // no button under this point
}

void MenuState::handleEvent(const sf::Event& event) {
    // --- KEYBOARD INPUT ---
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Up) {
            // Wrap around: if on first button, jump to last.
            m_selectedIndex = (m_selectedIndex - 1 + m_buttons.size()) % m_buttons.size();
            refreshButtonVisuals();
        }
        else if (keyEvent->code == sf::Keyboard::Key::Down) {
            m_selectedIndex = (m_selectedIndex + 1) % m_buttons.size();
            refreshButtonVisuals();
        }
        else if (keyEvent->code == sf::Keyboard::Key::Enter) {
            // Fire the currently-selected button's callback.
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_buttons.size())) {
                m_buttons[m_selectedIndex].onActivate();
            }
        }
        else if (keyEvent->code == sf::Keyboard::Key::Escape) {
            // Escape from main menu = exit game (same as Exit button).
            m_manager->popState();
        }
    }

    // --- MOUSE MOVEMENT (hover) ---
    if (const auto* moveEvent = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(
            static_cast<float>(moveEvent->position.x),
            static_cast<float>(moveEvent->position.y)
        );
        int hovered = buttonAtPoint(mousePos);
        if (hovered != -1 && hovered != m_selectedIndex) {
            // Moving the mouse over a button also changes keyboard selection,
            // so the two input methods stay in sync.
            m_selectedIndex = hovered;
            refreshButtonVisuals();
        }
    }

    // --- MOUSE CLICK ---
    if (const auto* clickEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (clickEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f clickPos(
                static_cast<float>(clickEvent->position.x),
                static_cast<float>(clickEvent->position.y)
            );
            int clicked = buttonAtPoint(clickPos);
            if (clicked != -1) {
                m_buttons[clicked].onActivate();
            }
        }
    }
}

void MenuState::update(float /*dt*/) {
    // No per-frame logic yet. Later you could add things like:
    //   - Pulsing title animation
    //   - Background particle effects (falling snow!)
    // For now the menu is static.
}

void MenuState::draw(sf::RenderWindow& window) {
    window.draw(m_title);
    for (const auto& btn : m_buttons) {
        window.draw(btn.background);
        window.draw(btn.text);
    }
}