#include "states/MultiplayerNameState.hpp"
#include "states/StateManager.hpp"
#include "states/PlayState.hpp"
#include "audio/AudioManager.hpp"
#include <iostream>

void MultiplayerNameState::onEnter() {
    AudioManager::get().playMenuMusic();
    if (m_font.openFromFile("assets/fonts/PressStart2P-Regular.ttf"))
        m_fontLoaded = true;
    if (m_bgTexture.loadFromFile("assets/sprites/leaderboard_bg.png")) {
        m_bg = new sf::Sprite(m_bgTexture);
        auto sz = m_bgTexture.getSize();
        if (sz.x > 0 && sz.y > 0)
            m_bg->setScale({W/(float)sz.x, H/(float)sz.y});
    }
    m_p2Name.clear();
}

void MultiplayerNameState::handleEvent(const sf::Event& event) {
    // Text input
    if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
        uint32_t c = te->unicode;
        if (c == '\b' || c == 8) {               // backspace
            if (!m_p2Name.empty()) m_p2Name.pop_back();
        } else if (c == '\r' || c == '\n') {     // enter = confirm
            if (!m_p2Name.empty()) {
                m_manager->getProgress().player2Name = m_p2Name;
                m_manager->replaceState(new PlayState(0, 1));
            }
        } else if (c >= 32 && c < 127 && m_p2Name.size() < 16) {
            m_p2Name += static_cast<char>(c);
        }
    }

    // Start button click
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp{(float)mb->position.x, (float)mb->position.y};
            if (m_startBtnBounds.contains(mp) && !m_p2Name.empty()) {
                m_manager->getProgress().player2Name = m_p2Name;
                m_manager->replaceState(new PlayState(0, 1));
            }
        }
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape)
            m_manager->popState();
        if (kp->code == sf::Keyboard::Key::Enter && !m_p2Name.empty()) {
            m_manager->getProgress().player2Name = m_p2Name;
            m_manager->replaceState(new PlayState(0, 1));
        }
    }
}

void MultiplayerNameState::drawCapsuleButton(sf::RenderWindow& win,
    float x, float y, float bw, float bh, sf::Color fill, const std::string& label)
{
    float r = bh / 2.f;
    sf::CircleShape lc(r), rc(r);
    sf::RectangleShape mid({bw - bh, bh});
    lc.setPosition({x, y});
    mid.setPosition({x + r, y});
    rc.setPosition({x + bw - bh, y});
    lc.setFillColor(fill); mid.setFillColor(fill); rc.setFillColor(fill);
    win.draw(lc); win.draw(mid); win.draw(rc);

    if (m_fontLoaded) {
        sf::Text t(m_font, label, 12);
        t.setFillColor(sf::Color::White);
        t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(1.5f);
        auto tb = t.getLocalBounds();
        t.setPosition({x + (bw - tb.size.x)/2.f - tb.position.x,
                       y + (bh - tb.size.y)/2.f - tb.position.y});
        win.draw(t);
    }
}

void MultiplayerNameState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(5, 10, 25));
    if (m_bg) window.draw(*m_bg);

    // Dark overlay panel
    sf::RectangleShape panel({500.f, 320.f});
    panel.setPosition({150.f, 140.f});
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setOutlineColor(sf::Color(80, 160, 255, 200));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    if (!m_fontLoaded) return;

    // Title
    { sf::Text t(m_font, "MULTIPLAYER", 16);
      t.setFillColor(sf::Color::Yellow);
      t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      auto tb = t.getLocalBounds();
      t.setPosition({(W - tb.size.x)/2.f - tb.position.x, 160.f});
      window.draw(t); }

    // P1 label (read-only)
    std::string p1name = m_manager->getCurrentUserName();
    if (p1name.empty()) p1name = "PLAYER 1";
    { sf::Text lbl(m_font, "PLAYER 1:", 9);
      lbl.setFillColor(sf::Color(100, 200, 255));
      lbl.setOutlineColor(sf::Color::Black); lbl.setOutlineThickness(1.f);
      lbl.setPosition({170.f, 230.f}); window.draw(lbl); }
    { sf::Text t(m_font, p1name, 10);
      t.setFillColor(sf::Color::White);
      t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(1.f);
      t.setPosition({170.f, 250.f}); window.draw(t); }

    // P2 label
    { sf::Text lbl(m_font, "PLAYER 2 NAME:", 9);
      lbl.setFillColor(sf::Color(255, 160, 80));
      lbl.setOutlineColor(sf::Color::Black); lbl.setOutlineThickness(1.f);
      lbl.setPosition({170.f, 295.f}); window.draw(lbl); }

    // P2 input box
    sf::RectangleShape box({460.f, 34.f});
    box.setPosition({170.f, 315.f});
    box.setFillColor(sf::Color(20, 20, 40, 220));
    box.setOutlineColor(sf::Color(255, 160, 80));
    box.setOutlineThickness(2.f);
    window.draw(box);

    std::string display = m_p2Name + (m_active ? "|" : "");
    { sf::Text t(m_font, display.empty() ? "Type name..." : display, 10);
      t.setFillColor(m_p2Name.empty() ? sf::Color(100,100,100) : sf::Color::White);
      t.setPosition({178.f, 323.f}); window.draw(t); }

    // Hint
    { sf::Text hint(m_font, "Press ENTER or click START", 8);
      hint.setFillColor(sf::Color(180, 180, 180));
      auto hb = hint.getLocalBounds();
      hint.setPosition({(W - hb.size.x)/2.f - hb.position.x, 370.f});
      window.draw(hint); }

    // START button
    float bw = 160.f, bh = 36.f, bx = (W - bw)/2.f, by = 405.f;
    m_startBtnBounds = {{bx, by}, {bw, bh}};
    sf::Color btnCol = m_p2Name.empty() ? sf::Color(60,60,60) : sf::Color(30,120,50);
    drawCapsuleButton(window, bx, by, bw, bh, btnCol, "START GAME");
}