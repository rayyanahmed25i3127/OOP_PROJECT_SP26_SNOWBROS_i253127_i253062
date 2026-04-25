#include "states/SignupState.hpp"
#include "states/StateManager.hpp"
#include "states/LoginState.hpp"
#include <iostream>
#include <cmath>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  Layout constants — same proportions as LoginState
// ─────────────────────────────────────────────────────────────────────────────
const float SignupState::BTN_X      = 260.f;
const float SignupState::BTN_W      = 280.f;
const float SignupState::BTN_H      =  55.f;
const float SignupState::REGISTER_Y = 350.f;
const float SignupState::BACK_Y     = 425.f;


// ─────────────────────────────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────
SignupState::SignupState()
    : m_bgSprite(nullptr),
      m_bgLoaded(false),
      m_typingUsername(true),
      m_typingPassword(false),
      m_titleText(nullptr),
      m_usernameText(nullptr),
      m_passwordText(nullptr),
      m_infoText(nullptr),
      m_registerText(nullptr),
      m_backText(nullptr),
      m_tabHintText(nullptr),
      m_registerHovered(false),
      m_backHovered(false)
{}

SignupState::~SignupState()
{
    delete m_bgSprite;
    delete m_titleText;
    delete m_usernameText;
    delete m_passwordText;
    delete m_infoText;
    delete m_registerText;
    delete m_backText;
    delete m_tabHintText;
}


// ─────────────────────────────────────────────────────────────────────────────
//  drawCapsule — single TriangleFan, no seam, no vector, no optional
//
//  Vertex count = 2 * SEGS + 4 = 60  →  array size 64.
// ─────────────────────────────────────────────────────────────────────────────
void SignupState::drawCapsule(sf::RenderWindow& window,
                               float x, float y,
                               float width, float height,
                               sf::Color fillColor,
                               sf::Color outlineColor,
                               float     outlineThickness)
{
    static const int SEGS      = 28;
    static const int MAX_VERTS = 2 * SEGS + 4;

    auto build = [&](float bx, float by,
                      float bw, float bh,
                      sf::Color col,
                      sf::Vertex out[]) -> int
    {
        float r    = bh / 2.f;
        float lcx  = bx + r;
        float rcx  = bx + bw - r;
        float midY = by + r;
        int   idx  = 0;

        out[idx++] = {{ bx + bw / 2.f, midY }, col};

        for (int i = 0; i <= SEGS; ++i) {
            float a = -3.14159265f / 2.f
                    +  3.14159265f * static_cast<float>(i) / SEGS;
            out[idx++] = {{ rcx + r * std::cos(a), midY + r * std::sin(a) }, col};
        }
        for (int i = 0; i <= SEGS; ++i) {
            float a = 3.14159265f / 2.f
                    + 3.14159265f * static_cast<float>(i) / SEGS;
            out[idx++] = {{ lcx + r * std::cos(a), midY + r * std::sin(a) }, col};
        }

        out[idx++] = out[1];
        return idx;
    };

    sf::Vertex verts[MAX_VERTS];
    int        cnt = 0;

    // 1. Outline — expanded capsule drawn first
    if (outlineThickness > 0.f && outlineColor.a > 0)
    {
        float ot = outlineThickness;
        cnt = build(x - ot, y - ot,
                    width + 2.f * ot, height + 2.f * ot,
                    outlineColor, verts);
        window.draw(verts, static_cast<std::size_t>(cnt),
                    sf::PrimitiveType::TriangleFan);
    }

    // 2. Fill
    cnt = build(x, y, width, height, fillColor, verts);
    window.draw(verts, static_cast<std::size_t>(cnt),
                sf::PrimitiveType::TriangleFan);
}


// ─────────────────────────────────────────────────────────────────────────────
//  drawInputField — capsule-shaped input box (matches LoginState exactly)
// ─────────────────────────────────────────────────────────────────────────────
void SignupState::drawInputField(sf::RenderWindow& window,
                                  float x, float y,
                                  float width, float height,
                                  bool  active)
{
    sf::Color fill(10, 20, 50, 200);
    sf::Color border = active
                     ? sf::Color(160, 220, 255, 255)   // bright ice-blue
                     : sf::Color( 70, 110, 160, 200);  // dim blue

    drawCapsule(window, x, y, width, height, fill, border, 2.f);
}


// ─────────────────────────────────────────────────────────────────────────────
//  onEnter
// ─────────────────────────────────────────────────────────────────────────────
void SignupState::onEnter()
{
    // ── Background (same image as LoginState) ─────────────────────────────────
    if (!m_bgTexture.loadFromFile("assets/sprites/login_bg.png")) {
        std::cerr << "Failed to load login_bg.png\n";
        m_bgLoaded = false;
    } else {
        m_bgLoaded = true;
        delete m_bgSprite;
        m_bgSprite = new sf::Sprite(m_bgTexture);

        sf::Vector2u tex = m_bgTexture.getSize();
        sf::Vector2u win = m_window->getSize();
        m_bgSprite->setScale({
            static_cast<float>(win.x) / tex.x,
            static_cast<float>(win.y) / tex.y
        });
    }

    // ── Font ─────────────────────────────────────────────────────────────────
    if (!m_font.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf"))
        std::cerr << "Font failed\n";

    // ── Title ─────────────────────────────────────────────────────────────────
    m_titleText = new sf::Text(m_font);
    m_titleText->setString("CREATE ACCOUNT");
    m_titleText->setCharacterSize(40);
    m_titleText->setFillColor(sf::Color(180, 230, 255));

    sf::FloatRect tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({ tb.position.x + tb.size.x / 2.f,
                             tb.position.y + tb.size.y / 2.f });
    m_titleText->setPosition({ 400.f, 70.f });

    // ── Username field text ───────────────────────────────────────────────────
    m_usernameText = new sf::Text(m_font);
    m_usernameText->setCharacterSize(22);
    m_usernameText->setFillColor(sf::Color::White);
    m_usernameText->setPosition({ 275.f, 207.f });

    // ── Password field text ───────────────────────────────────────────────────
    m_passwordText = new sf::Text(m_font);
    m_passwordText->setCharacterSize(22);
    m_passwordText->setFillColor(sf::Color::White);
    m_passwordText->setPosition({ 275.f, 282.f });

    // ── Info / feedback message ───────────────────────────────────────────────
    m_infoText = new sf::Text(m_font);
    m_infoText->setCharacterSize(20);
    m_infoText->setFillColor(sf::Color(255, 200, 100));   // amber
    m_infoText->setString("");
    m_infoText->setPosition({ 400.f, 315.f });

    // ── Tab hint ──────────────────────────────────────────────────────────────
    m_tabHintText = new sf::Text(m_font);
    m_tabHintText->setString("Tab to switch field    Enter to confirm");
    m_tabHintText->setCharacterSize(14);
    m_tabHintText->setFillColor(sf::Color(120, 160, 200, 180));

    sf::FloatRect hb = m_tabHintText->getLocalBounds();
    m_tabHintText->setOrigin({ hb.position.x + hb.size.x / 2.f, 0.f });
    m_tabHintText->setPosition({ 400.f, 500.f });

    // ── REGISTER button label ─────────────────────────────────────────────────
    // Always create with 'new' first — then call setters on it.
    m_registerText = new sf::Text(m_font);
    m_registerText->setString("REGISTER");
    m_registerText->setCharacterSize(26);
    m_registerText->setFillColor(sf::Color::White);

    sf::FloatRect rb = m_registerText->getLocalBounds();
    m_registerText->setOrigin({ rb.position.x + rb.size.x / 2.f,
                                rb.position.y + rb.size.y / 2.f });
    m_registerText->setPosition({ BTN_X + BTN_W / 2.f,
                                  REGISTER_Y + BTN_H / 2.f });

    // ── BACK button label ─────────────────────────────────────────────────────
    m_backText = new sf::Text(m_font);
    m_backText->setString("BACK");
    m_backText->setCharacterSize(26);
    m_backText->setFillColor(sf::Color::White);

    sf::FloatRect bb = m_backText->getLocalBounds();
    m_backText->setOrigin({ bb.position.x + bb.size.x / 2.f,
                            bb.position.y + bb.size.y / 2.f });
    m_backText->setPosition({ BTN_X + BTN_W / 2.f,
                              BACK_Y + BTN_H / 2.f });
}


// ─────────────────────────────────────────────────────────────────────────────
//  handleEvent
// ─────────────────────────────────────────────────────────────────────────────
void SignupState::handleEvent(const sf::Event& event)
{
    // ── Text input ────────────────────────────────────────────────────────────
    if (const auto* te = event.getIf<sf::Event::TextEntered>())
    {
        if (te->unicode == 8)   // backspace
        {
            if (m_typingUsername && !m_username.empty())
                m_username.pop_back();
            else if (m_typingPassword && !m_password.empty())
                m_password.pop_back();
        }
        else if (te->unicode >= 32 && te->unicode < 128)
        {
            char c = static_cast<char>(te->unicode);
            if      (m_typingUsername) m_username += c;
            else if (m_typingPassword) m_password += c;
        }
    }

    // ── Keyboard ──────────────────────────────────────────────────────────────
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Tab)
        {
            m_typingUsername = !m_typingUsername;
            m_typingPassword = !m_typingPassword;
        }

        if (key->code == sf::Keyboard::Key::Escape)
        {
            m_manager->replaceState(new LoginState());
        }

        if (key->code == sf::Keyboard::Key::Enter)
        {
            int id = m_manager->getUserManager().registerUser(m_username, m_password);

if (id != -1)
{
    m_infoText->setString("User created! Now login.");
    m_manager->replaceState(new LoginState());
}
else
{
    m_infoText->setString("User already exists!");
}
        }
    }

    // ── Mouse click ───────────────────────────────────────────────────────────
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mp(static_cast<float>(mouse->position.x),
                            static_cast<float>(mouse->position.y));

            // ── Click to focus a field ────────────────────────────────────────
            sf::FloatRect unameField({ 260.f, 195.f }, { 280.f, 45.f });
            sf::FloatRect passField ({ 260.f, 270.f }, { 280.f, 45.f });

            if (unameField.contains(mp))
            {
                m_typingUsername = true;
                m_typingPassword = false;
            }
            else if (passField.contains(mp))
            {
                m_typingUsername = false;
                m_typingPassword = true;
            }

            // ── REGISTER button ───────────────────────────────────────────────
            if (m_registerHovered)
            {
                if (!m_username.empty() && !m_password.empty())
                {
                    int id = m_manager->getUserManager().registerUser(m_username, m_password);
                    if (id != -1)
                    {
                        // Registration successful — go back to login page
                        m_manager->replaceState(new LoginState());
                    }
                    else
                    {
                        m_infoText->setString("Username already taken — try another.");
                    }
                }
                else
                {
                    m_infoText->setString("Please fill in both fields.");
                }
            }

            // ── BACK button ───────────────────────────────────────────────────
            if (m_backHovered)
            {
                m_manager->replaceState(new LoginState());
            }
        }
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  update
// ─────────────────────────────────────────────────────────────────────────────
void SignupState::update(float /*dt*/)
{
    // ── Refresh field strings ─────────────────────────────────────────────────
    if (m_usernameText)
        m_usernameText->setString("Name:     " + m_username);

    if (m_passwordText)
    {
        std::string stars(m_password.length(), '*');
        m_passwordText->setString("Password: " + stars);
    }

    // ── Hover detection ───────────────────────────────────────────────────────
    if (!m_window) return;

    sf::Vector2f mp = m_window->mapPixelToCoords(
                          sf::Mouse::getPosition(*m_window));

    m_registerHovered = sf::FloatRect({ BTN_X, REGISTER_Y }, { BTN_W, BTN_H }).contains(mp);
    m_backHovered     = sf::FloatRect({ BTN_X, BACK_Y     }, { BTN_W, BTN_H }).contains(mp);

    // ── Re-centre info text ───────────────────────────────────────────────────
    if (m_infoText)
    {
        sf::FloatRect ib = m_infoText->getLocalBounds();
        m_infoText->setOrigin({ ib.position.x + ib.size.x / 2.f, 0.f });
        m_infoText->setPosition({ 400.f, 315.f });
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  draw
// ─────────────────────────────────────────────────────────────────────────────
void SignupState::draw(sf::RenderWindow& window)
{
    window.clear(sf::Color(8, 14, 35));

    // ── 1. Background ─────────────────────────────────────────────────────────
    if (m_bgLoaded && m_bgSprite)
        window.draw(*m_bgSprite);

    // ── 2. Semi-transparent card ──────────────────────────────────────────────
    drawCapsule(window,
                200.f, 40.f, 400.f, 530.f,
                sf::Color(15, 30, 70, 170),
                sf::Color(80, 140, 200, 100),
                1.f);

    // ── 3. Title ──────────────────────────────────────────────────────────────
    if (m_titleText) window.draw(*m_titleText);

    // ── 4. Username input field + text ────────────────────────────────────────
    drawInputField(window, 260.f, 195.f, 280.f, 45.f, m_typingUsername);
    if (m_usernameText) window.draw(*m_usernameText);

    // ── 5. Password input field + text ────────────────────────────────────────
    drawInputField(window, 260.f, 270.f, 280.f, 45.f, m_typingPassword);
    if (m_passwordText) window.draw(*m_passwordText);

    // ── 6. Info / error message ───────────────────────────────────────────────
    if (m_infoText) window.draw(*m_infoText);

    // ── 7. REGISTER button ────────────────────────────────────────────────────
    // Shadow
    drawCapsule(window,
                BTN_X + 3.f, REGISTER_Y + 4.f, BTN_W, BTN_H,
                sf::Color(10, 30, 90, 200));

    sf::Color regFill = m_registerHovered
                      ? sf::Color( 80, 190, 255, 255)
                      : sf::Color( 50, 140, 220, 255);

    drawCapsule(window,
                BTN_X, REGISTER_Y, BTN_W, BTN_H,
                regFill,
                sf::Color::White,   // white border (same colour as font)
                3.f);

    // Gloss sheen
    drawCapsule(window,
                BTN_X + 20.f, REGISTER_Y + 5.f,
                BTN_W - 40.f, BTN_H * 0.35f,
                sf::Color(200, 235, 255, 60));

    if (m_registerText)
    {
        m_registerText->setFillColor(sf::Color::White);
        window.draw(*m_registerText);
    }

    // ── 8. BACK button ────────────────────────────────────────────────────────
    drawCapsule(window,
                BTN_X + 3.f, BACK_Y + 4.f, BTN_W, BTN_H,
                sf::Color(10, 30, 90, 200));

    sf::Color backFill = m_backHovered
                       ? sf::Color( 80, 190, 255, 255)
                       : sf::Color( 50, 140, 220, 255);

    drawCapsule(window,
                BTN_X, BACK_Y, BTN_W, BTN_H,
                backFill,
                sf::Color::White,
                3.f);

    drawCapsule(window,
                BTN_X + 20.f, BACK_Y + 5.f,
                BTN_W - 40.f, BTN_H * 0.35f,
                sf::Color(200, 235, 255, 60));

    if (m_backText)
    {
        m_backText->setFillColor(sf::Color::White);
        window.draw(*m_backText);
    }

    // ── 9. Tab hint ───────────────────────────────────────────────────────────
    if (m_tabHintText) window.draw(*m_tabHintText);
}