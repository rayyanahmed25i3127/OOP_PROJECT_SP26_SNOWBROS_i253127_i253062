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
    const float HITBOX_WIDTH   = 36.f;   // ~65% of sprite width
    const float HITBOX_HEIGHT  = 60.f;   // ~88% of sprite height

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
    , jumpForce(-550.f)
    , gravity(800.f)
    , onGround(false)
    , m_facingRight(true)
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
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }
}

void Player::applyGravity(float dt) {
    velocity.y += gravity * dt;
}

void Player::update(float dt) {
    handleInput();
    applyGravity(dt);

    // Apply velocity to position.
    // Collision resolution happens AFTER this, in CollisionDetector,
    // called by PlayState. Do NOT do any ground/wall/platform checks here.
    position += velocity * dt;

    // Hit-box tracks position
    // Hit-box tracks position (with offset from sprite origin)
    hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };

    // Update sprite — flip horizontally based on facing direction.
    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float absScaleX = PLAYER_WIDTH  / static_cast<float>(texSize.x);
        float absScaleY = PLAYER_HEIGHT / static_cast<float>(texSize.y);

        if (m_facingRight) {
            m_sprite.setScale({absScaleX, absScaleY});
            m_sprite.setPosition(position);
        } else {
            m_sprite.setScale({-absScaleX, absScaleY});
            m_sprite.setPosition({position.x + PLAYER_WIDTH, position.y});
        }
    }
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(m_sprite);
}

// NEW: add this at the very bottom
void Player::setPosition(sf::Vector2f pos) {
    position = pos;
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
    m_sprite.setPosition(pos);
}