#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

namespace {
    // Visual size — what's drawn on screen
    const float PLAYER_WIDTH  = 56.f;
    const float PLAYER_HEIGHT = 68.f;

    // Collision hitbox — smaller than visual. This is what physics uses.
    // Narrower so you can squeeze past platform edges; slightly shorter
    // so the sprite appears to touch surfaces instead of floating.
    const float HITBOX_WIDTH   = 24.f;   // tighter   // ~65% of sprite width
    const float HITBOX_HEIGHT  = 57.f;   // ~88% of sprite height

    // Offset of hitbox WITHIN the sprite rectangle.
    // The sprite's visible character occupies roughly the horizontal center
    // and slightly below the top, so we offset the hitbox accordingly.
    const float HITBOX_OFFSET_X = (PLAYER_WIDTH  - HITBOX_WIDTH)  / 2.f;  // = 10
    const float HITBOX_OFFSET_Y =  PLAYER_HEIGHT - HITBOX_HEIGHT;         // = 8 (align bottom)
}




Player::Player(sf::Vector2f pos)
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
    , m_throwInterval(0.18f)   // ~5.5 shots/sec — arcade feel
    , m_wantsToThrow(false)
{
    if (!m_texture.loadFromFile("assets/sprites/player_blue_idle.png")) {
        std::cerr << "[Player] Failed to load player_blue_idle.png\n";
    } else {
        m_sprite.setTexture(m_texture, true);
        auto texSize = m_texture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float scaleX = PLAYER_WIDTH  / static_cast<float>(texSize.x);
            float scaleY = PLAYER_HEIGHT / static_cast<float>(texSize.y);
            m_sprite.setScale({scaleX, scaleY});
        }
    }

    m_sprite.setPosition(pos);

    // Hitbox sits INSIDE the sprite rectangle, offset from its top-left.
    // The hitbox's bottom is aligned with the sprite's bottom (so the player
    // "feet" match visually when landing on platforms).
    hitBox.size = { HITBOX_WIDTH, HITBOX_HEIGHT };
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

    // Jump: W or Up Arrow (spec §14). Space is now throw-only.
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }

    // Throw snowball: Space (or J as spec-valid alternate).
    // We only FLAG the intent here — PlayState spawns the actual
    // snowball so Player stays decoupled from the projectile array.
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        && m_throwCooldown <= 0.f) {
        m_wantsToThrow = true;
    }
}

void Player::applyGravity(float dt) {
    velocity.y += gravity * dt;
}

void Player::update(float dt) {

    if (m_throwCooldown > 0.f) m_throwCooldown -= dt;
    // --- Invincibility countdown + blink toggle ---
    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= dt;

        // Flip visibility every 0.1 seconds for the blink effect.
        // Using floor(timer * 10) parity keeps blinking framerate-stable.
        int phase = static_cast<int>(m_invincibleTimer * 10.f);
        m_blinkVisible = (phase % 2 == 0);

        if (m_invincibleTimer <= 0.f) {
            m_invincibleTimer = 0.f;
            m_blinkVisible = true;  // ensure visible when invincibility ends
        }
    }

    handleInput();
    applyGravity(dt);

    position += velocity * dt;
    hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };

    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float absScaleX = PLAYER_WIDTH  / static_cast<float>(texSize.x);
        float absScaleY = PLAYER_HEIGHT / static_cast<float>(texSize.y);

        m_sprite.setOrigin({ static_cast<float>(texSize.x) / 2.f, 0.f });

        // Source sprite is drawn facing LEFT, so flip X when facing right.
        if (m_facingRight) {
            m_sprite.setScale({-absScaleX, absScaleY});
        } else {
            m_sprite.setScale({ absScaleX, absScaleY});
        }

        m_sprite.setPosition({ position.x + PLAYER_WIDTH / 2.f, position.y });
    }
}
void Player::draw(sf::RenderWindow& window) {
    // During invincibility, blink the sprite. Hit-box debug outline (drawn
    // by PlayState) is unaffected — always shows actual collision shape.
    if (m_invincibleTimer > 0.f && !m_blinkVisible) return;
    window.draw(m_sprite);
}

// NEW: add this at the very bottom
void Player::setPosition(sf::Vector2f pos) {
    position = pos;
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
    // Sprite origin is at its horizontal center, so offset position accordingly.
    m_sprite.setPosition({ pos.x + PLAYER_WIDTH / 2.f, pos.y });
}

void Player::loseLife() {
    if (m_invincibleTimer > 0.f) return;   // already in grace period
    if (m_lives <= 0) return;               // already dead

    --m_lives;
    m_invincibleTimer = 3.0f;
    m_blinkVisible = true;
    // Respawn position is decided by the game state (PlayState owns the
    // spawn coords). PlayState calls respawn() after loseLife() when
    // m_lives > 0.
}

void Player::respawn(sf::Vector2f spawnPos) {
    setPosition(spawnPos);
    velocity = { 0.f, 0.f };
    onGround = false;
    m_facingRight = true;
}