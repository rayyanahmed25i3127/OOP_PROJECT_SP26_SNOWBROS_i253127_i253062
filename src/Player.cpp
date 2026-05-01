#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

namespace {
    const float PLAYER_WIDTH  = 56.f;
    const float PLAYER_HEIGHT = 68.f;

    const float HITBOX_WIDTH   = 24.f;
    const float HITBOX_HEIGHT  = 57.f;
    const float HITBOX_OFFSET_X = (PLAYER_WIDTH  - HITBOX_WIDTH)  / 2.f;
    const float HITBOX_OFFSET_Y =  PLAYER_HEIGHT - HITBOX_HEIGHT;

    // ── Character sprite map ────────────────────────────────────────────
    // Index matches PlayState::m_characterIndex (0=Blue, 1=Red, 2=Modi)
    const char* PLAYER_SPRITES[3] = {
        "assets/sprites/player_blue_idle.png",  // 0 – Nick (blue)
        "assets/sprites/player_red_idle.png",   // 1 – Tom  (red)
        "assets/sprites/player_modi_idle.png"   // 2 – Modi Ji
    };
    const int NUM_PLAYER_SPRITES = 3;
}

// ── ctor ────────────────────────────────────────────────────────────────────
Player::Player(sf::Vector2f pos, int characterIndex)
    : Entity(pos)
    , m_sprite(m_texture)
    , m_lives(2)
    , m_invincibleTimer(0.f)
    , m_blinkVisible(true)
    , m_throwCooldown(0.f)
    , m_throwInterval(0.18f)
    , m_wantsToThrow(false)
    , speed(200.f)
    , jumpForce(-450.f)
    , gravity(800.f)
    , onGround(false)
    , m_facingRight(true)
    , m_balloonMode(false)
    , m_balloonGravity(-50.f)
{
    // Pick sprite path from the character index (clamp to safe range)
    int idx = (characterIndex >= 0 && characterIndex < NUM_PLAYER_SPRITES)
              ? characterIndex : 0;
    const char* spritePath = PLAYER_SPRITES[idx];

    if (!m_texture.loadFromFile(spritePath)) {
        // Graceful fallback: try blue as default
        std::cerr << "[Player] Failed to load " << spritePath
                  << " — falling back to player_blue_idle.png\n";
        m_texture.loadFromFile("assets/sprites/player_blue_idle.png");
    } else {
        std::cout << "[Player] Loaded sprite: " << spritePath << "\n";
    }

    m_sprite.setTexture(m_texture, true);
    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = PLAYER_WIDTH  / static_cast<float>(texSize.x);
        float scaleY = PLAYER_HEIGHT / static_cast<float>(texSize.y);
        m_sprite.setScale({scaleX, scaleY});
    }

    m_sprite.setPosition(pos);

    hitBox.size     = { HITBOX_WIDTH,  HITBOX_HEIGHT };
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
}

void Player::handleInput() {
    velocity.x = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        velocity.x = -speed;
        m_facingRight = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        velocity.x = speed;
        m_facingRight = true;
    }

    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }

    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        && m_throwCooldown <= 0.f) {
        m_wantsToThrow = true;
    }
}

void Player::applyGravity(float dt) {
    if (m_balloonMode) {
        velocity.y += m_balloonGravity * dt;
    } else {
        velocity.y += gravity * dt;
    }
}

void Player::setSpeedMultiplier(float multiplier) {
    const float BASE_SPEED = 200.f;
    speed = BASE_SPEED * multiplier;
}

void Player::update(float dt) {
    if (m_throwCooldown > 0.f) m_throwCooldown -= dt;

    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= dt;
        int phase = static_cast<int>(m_invincibleTimer * 10.f);
        m_blinkVisible = (phase % 2 == 0);
        if (m_invincibleTimer <= 0.f) {
            m_invincibleTimer = 0.f;
            m_blinkVisible = true;
        }
    }

    handleInput();
    applyGravity(dt);

    position += velocity * dt;
    hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };

    if (m_balloonMode) {
        const float TOP_BOUNDARY    = 15.f;
        const float BOTTOM_BOUNDARY = 585.f;
        const float LEFT_BOUNDARY   = 30.f;
        const float RIGHT_BOUNDARY  = 770.f;

        if (position.y < TOP_BOUNDARY) {
            position.y  = TOP_BOUNDARY;
            velocity.y  = -velocity.y;
        }
        if (position.y + PLAYER_HEIGHT > BOTTOM_BOUNDARY) {
            position.y  = BOTTOM_BOUNDARY - PLAYER_HEIGHT;
            velocity.y  = -velocity.y;
        }
        if (position.x < LEFT_BOUNDARY) {
            position.x  = LEFT_BOUNDARY;
            velocity.x  = -velocity.x;
        }
        if (position.x + PLAYER_WIDTH > RIGHT_BOUNDARY) {
            position.x  = RIGHT_BOUNDARY - PLAYER_WIDTH;
            velocity.x  = -velocity.x;
        }

        hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };
    }

    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float absScaleX = PLAYER_WIDTH  / static_cast<float>(texSize.x);
        float absScaleY = PLAYER_HEIGHT / static_cast<float>(texSize.y);

        m_sprite.setOrigin({ static_cast<float>(texSize.x) / 2.f, 0.f });

        if (m_facingRight) {
            m_sprite.setScale({-absScaleX, absScaleY});
        } else {
            m_sprite.setScale({ absScaleX, absScaleY});
        }

        m_sprite.setPosition({ position.x + PLAYER_WIDTH / 2.f, position.y });
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (m_invincibleTimer > 0.f && !m_blinkVisible) return;
    window.draw(m_sprite);
}

void Player::setPosition(sf::Vector2f pos) {
    position = pos;
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
    m_sprite.setPosition({ pos.x + PLAYER_WIDTH / 2.f, pos.y });
}

void Player::loseLife() {
    if (m_invincibleTimer > 0.f) return;
    if (m_lives <= 0) return;

    --m_lives;
    m_invincibleTimer = 3.0f;
    m_blinkVisible = true;
}

void Player::respawn(sf::Vector2f spawnPos) {
    setPosition(spawnPos);
    velocity    = { 0.f, 0.f };
    onGround    = false;
    m_facingRight = true;
}