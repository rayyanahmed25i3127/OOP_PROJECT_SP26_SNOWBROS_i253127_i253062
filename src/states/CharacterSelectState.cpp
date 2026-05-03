#include "states/CharacterSelectState.hpp"
#include "states/StateManager.hpp"
#include "audio/AudioManager.hpp"
#include "states/PlayState.hpp"
#include <iostream>
namespace {
    const char* CHAR_FILES[CharacterSelectState::NUM_CHARS] = {
        "assets/sprites/char_blue.png",
        "assets/sprites/char_red.png",
        "assets/sprites/char_modi.png"
    };
    const char* CHAR_NAMES[CharacterSelectState::NUM_CHARS] = {
        "NICK",
        "TOM",
        "MODI JI"
    };

    const sf::Color CARD_IDLE  (10,  20,  45,  210);
    const sf::Color CARD_HOVER (40,  80,  160, 230);
    const sf::Color CARD_BORDER(80,  160, 255, 200);
    const sf::Color CARD_BORDER_HOVER(255, 220, 60, 255);
}
CharacterSelectState::CharacterSelectState()
    : m_bg(nullptr)
    , m_fontLoaded(false)
    , m_title(nullptr)
    , m_hoveredIndex(-1)
{
    for (int i = 0; i < NUM_CHARS; ++i) {
        m_sprites[i] = nullptr;
        m_labels[i]  = nullptr;
    }
}

CharacterSelectState::~CharacterSelectState() {
    delete m_bg;
    delete m_title;
    for (int i = 0; i < NUM_CHARS; ++i) {
        delete m_sprites[i];
        delete m_labels[i];
    }
}

void CharacterSelectState::onEnter() {
    AudioManager::get().playMenuMusic();
    std::cout << "[CharSelect] Entering character selection\n";

    // Background
    if (m_bgTexture.loadFromFile("assets/sprites/leaderboard_bg.png")) {
        m_bg = new sf::Sprite(m_bgTexture);
        auto sz = m_bgTexture.getSize();
        if (sz.x > 0 && sz.y > 0) {
            m_bg->setScale({ WINDOW_W / static_cast<float>(sz.x),
                             WINDOW_H / static_cast<float>(sz.y) });
        }
    } else {
        std::cerr << "[CharSelect] Could not load leaderboard_bg.png\n";
    }

    // Font
    if (m_font.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        m_fontLoaded = true;
    } else {
        std::cerr << "[CharSelect] Could not load font\n";
    }

    // Character textures + sprites
    for (int i = 0; i < NUM_CHARS; ++i) {
        if (!m_textures[i].loadFromFile(CHAR_FILES[i])) {
            std::cerr << "[CharSelect] Failed to load: " << CHAR_FILES[i] << "\n";
        }
        m_sprites[i] = new sf::Sprite(m_textures[i]);
    }

    // Title
    if (m_fontLoaded) {
        m_title = new sf::Text(m_font, "SELECT YOUR CHARACTER", 16);
        m_title->setFillColor(sf::Color::White);
        m_title->setOutlineColor(sf::Color::Black);
        m_title->setOutlineThickness(3.f);
        auto tb = m_title->getLocalBounds();
        m_title->setPosition({
            (WINDOW_W - tb.size.x) / 2.f - tb.position.x,
            30.f
        });
    }

    buildLayout();
}
void CharacterSelectState::onExit() {
    std::cout << "[CharSelect] Exiting character selection\n";
}

void CharacterSelectState::buildLayout() {
    float totalW = NUM_CHARS * CARD_W + (NUM_CHARS - 1) * CARD_GAP;
    float startX = (WINDOW_W - totalW) / 2.f;

    for (int i = 0; i < NUM_CHARS; ++i) {
        float cardX = startX + i * (CARD_W + CARD_GAP);

        m_cards[i].setSize({ CARD_W, CARD_H });
        m_cards[i].setPosition({ cardX, CARD_TOP_Y });
        m_cards[i].setFillColor(CARD_IDLE);
        m_cards[i].setOutlineColor(CARD_BORDER);
        m_cards[i].setOutlineThickness(2.f);

        auto texSz = m_textures[i].getSize();
        if (texSz.x > 0 && texSz.y > 0 && m_sprites[i]) {
            float scl = IMG_SIZE / static_cast<float>(texSz.x);
            m_sprites[i]->setScale({ scl, scl });
            float imgX = cardX + (CARD_W - IMG_SIZE) / 2.f;
            float imgY = CARD_TOP_Y + IMG_PAD_TOP;
            m_sprites[i]->setPosition({ imgX, imgY });
        }

        m_bounds[i] = m_cards[i].getGlobalBounds();

        if (m_fontLoaded) {
            m_labels[i] = new sf::Text(m_font, CHAR_NAMES[i], 10);
            m_labels[i]->setFillColor(sf::Color::White);
            m_labels[i]->setOutlineColor(sf::Color::Black);
            m_labels[i]->setOutlineThickness(2.f);
            auto lb = m_labels[i]->getLocalBounds();
            float lx = cardX + (CARD_W - lb.size.x) / 2.f - lb.position.x;
            float ly  = CARD_TOP_Y + CARD_H - 32.f;
            m_labels[i]->setPosition({ lx, ly });
        }
    }
}
void CharacterSelectState::updateHover(sf::Vector2f mousePos) {
    m_hoveredIndex = -1;
    for (int i = 0; i < NUM_CHARS; ++i) {
        bool hit = m_bounds[i].contains(mousePos);
        if (hit) m_hoveredIndex = i;
        m_cards[i].setFillColor(hit ? CARD_HOVER  : CARD_IDLE);
        m_cards[i].setOutlineColor(hit ? CARD_BORDER_HOVER : CARD_BORDER);
        m_cards[i].setOutlineThickness(hit ? 3.f : 2.f);
    }
}

void CharacterSelectState::handleEvent(const sf::Event& event) {
    // Hover highlight
    if (const auto* mv = event.getIf<sf::Event::MouseMoved>()) {
        updateHover({ static_cast<float>(mv->position.x),
                      static_cast<float>(mv->position.y) });
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f pos{ static_cast<float>(mb->position.x),
                              static_cast<float>(mb->position.y) };
            for (int i = 0; i < NUM_CHARS; ++i) {
                if (m_bounds[i].contains(pos)) {
                    std::cout << "[CharSelect] Selected: " << CHAR_NAMES[i] << " → using NICK\n";

                    m_manager->popState();                 
                    m_manager->pushState(new PlayState(0)); 
                    return;
                }
            }
        }
    }


    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) {
            m_manager->popState();
        }
    }
}

void CharacterSelectState::update(float /*dt*/) {}

void CharacterSelectState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(5, 10, 25));

    if (m_bg) window.draw(*m_bg);

    {
        sf::RectangleShape banner({ WINDOW_W, 70.f });
        banner.setPosition({ 0.f, 15.f });
        banner.setFillColor(sf::Color(0, 0, 0, 140));
        window.draw(banner);
    }

    if (m_title) window.draw(*m_title);

    if (m_fontLoaded) {
        sf::Text hint(m_font, "CLICK A CHARACTER TO BEGIN", 8);
        hint.setFillColor(sf::Color(180, 200, 255, 200));
        auto hb = hint.getLocalBounds();
        hint.setPosition({ (WINDOW_W - hb.size.x) / 2.f - hb.position.x, 78.f });
        window.draw(hint);
    }

    for (int i = 0; i < NUM_CHARS; ++i) {
        window.draw(m_cards[i]);
        if (m_sprites[i]) window.draw(*m_sprites[i]);
        if (m_labels[i])  window.draw(*m_labels[i]);
    }

    // Arrow below hovered card
    if (m_hoveredIndex >= 0 && m_fontLoaded) {
        sf::Text arrow(m_font, "v", 12);
        arrow.setFillColor(sf::Color(255, 220, 60));
        arrow.setOutlineColor(sf::Color::Black);
        arrow.setOutlineThickness(2.f);
        auto ab = arrow.getLocalBounds();
        float cx = m_cards[m_hoveredIndex].getPosition().x + CARD_W / 2.f;
        arrow.setPosition({ cx - ab.size.x / 2.f, CARD_TOP_Y + CARD_H + 6.f });
        window.draw(arrow);
    }
}