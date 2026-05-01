#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

namespace {
    const float PLAYER_WIDTH  = 56.f;
    const float PLAYER_HEIGHT = 68.f;

    const float HITBOX_WIDTH    = 24.f;
    const float HITBOX_HEIGHT   = 57.f;
    const float HITBOX_OFFSET_X = (PLAYER_WIDTH  - HITBOX_WIDTH)  / 2.f;
    const float HITBOX_OFFSET_Y =  PLAYER_HEIGHT - HITBOX_HEIGHT;

    const char* PLAYER_SPRITES[3] = {
        "assets/sprites/player_blue_idle.png",
        "assets/sprites/player_red_idle.png",
        "assets/sprites/player_modi_idle.png"
    };
    const int NUM_PLAYER_SPRITES = 3;
}

Player::Player(sf::Vector2f pos, int characterIndex)
    : Entity(pos)
    , m_sprite(m_texture)
    , speed(200.f)
    , jumpForce(-450.f)
    , gravity(800.f)
    , onGround(false)
    , m_facingRight(true)
    , m_lives(2)
    , m_invincibleTimer(0.f)
    , m_blinkVisible(true)
    , m_throwCooldown(0.f)
    , m_throwInterval(0.18f)
    , m_wantsToThrow(false)
    , m_balloonMode(false)
    , m_balloonGravity(-50.f)
{
    int idx = (characterIndex >= 0 && characterIndex < NUM_PLAYER_SPRITES)
              ? characterIndex : 0;
    const char* spritePath = PLAYER_SPRITES[idx];

    if (!m_texture.loadFromFile(spritePath)) {
        std::cerr << "[Player] Failed to load " << spritePath
                  << " — falling back to player_blue_idle.png\n";
        m_texture.loadFromFile("assets/sprites/player_blue_idle.png");
    } else {
        std::cout << "[Player] Loaded sprite: " << spritePath << "\n";
    }

    m_sprite.setTexture(m_texture, true);
    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        m_sprite.setScale({ PLAYER_WIDTH  / static_cast<float>(texSize.x),
                            PLAYER_HEIGHT / static_cast<float>(texSize.y) });
    }
    m_sprite.setPosition(pos);

    hitBox.size     = { HITBOX_WIDTH, HITBOX_HEIGHT };
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
}

// ── addLife ──────────────────────────────────────────────────────────────────
// Capped at MAX_LIVES (3). Returns true if a life was actually added.
bool Player::addLife() {
    if (m_lives >= MAX_LIVES) {
        std::cout << "[Player] addLife() ignored — already at MAX_LIVES ("
                  << MAX_LIVES << ")\n";
        return false;
    }
    ++m_lives;
    std::cout << "[Player] addLife() → lives now " << m_lives << "\n";
    return true;
}

void Player::handleInput() {
    velocity.x = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        velocity.x = -speed;  m_facingRight = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        velocity.x =  speed;  m_facingRight = true;
    }
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
      || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        && onGround) {
        velocity.y = jumpForce;
        onGround   = false;
    }
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
      || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        && m_throwCooldown <= 0.f) {
        m_wantsToThrow = true;
    }
}

void Player::applyGravity(float dt) {
    velocity.y += (m_balloonMode ? m_balloonGravity : gravity) * dt;
}

void Player::setSpeedMultiplier(float multiplier) {
    speed = 200.f * multiplier;
}

void Player::update(float dt) {
    if (m_throwCooldown > 0.f) m_throwCooldown -= dt;

    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= dt;
        m_blinkVisible = (static_cast<int>(m_invincibleTimer * 10.f) % 2 == 0);
        if (m_invincibleTimer <= 0.f) { m_invincibleTimer = 0.f; m_blinkVisible = true; }
    }

    handleInput();
    applyGravity(dt);
    position += velocity * dt;
    hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };

    if (m_balloonMode) {
        const float TOP = 15.f, BOT = 585.f, LEFT = 30.f, RIGHT = 770.f;
        if (position.y < TOP)                       { position.y  = TOP;                       velocity.y = -velocity.y; }
        if (position.y + PLAYER_HEIGHT > BOT)       { position.y  = BOT - PLAYER_HEIGHT;       velocity.y = -velocity.y; }
        if (position.x < LEFT)                      { position.x  = LEFT;                      velocity.x = -velocity.x; }
        if (position.x + PLAYER_WIDTH > RIGHT)      { position.x  = RIGHT - PLAYER_WIDTH;      velocity.x = -velocity.x; }
        hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };
    }

    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float ax = PLAYER_WIDTH  / static_cast<float>(texSize.x);
        float ay = PLAYER_HEIGHT / static_cast<float>(texSize.y);
        m_sprite.setOrigin({ static_cast<float>(texSize.x) / 2.f, 0.f });
        m_sprite.setScale({ m_facingRight ? -ax : ax, ay });
        m_sprite.setPosition({ position.x + PLAYER_WIDTH / 2.f, position.y });
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (m_invincibleTimer > 0.f && !m_blinkVisible) return;
    window.draw(m_sprite);
}

void Player::setPosition(sf::Vector2f pos) {
    position        = pos;
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
    m_sprite.setPosition({ pos.x + PLAYER_WIDTH / 2.f, pos.y });
}

void Player::loseLife() {
    if (m_invincibleTimer > 0.f || m_lives <= 0) return;
    --m_lives;
    m_invincibleTimer = 3.0f;
    m_blinkVisible    = true;
}

void Player::respawn(sf::Vector2f spawnPos) {
    setPosition(spawnPos);
    velocity      = { 0.f, 0.f };
    onGround      = false;
    m_facingRight = true;
}