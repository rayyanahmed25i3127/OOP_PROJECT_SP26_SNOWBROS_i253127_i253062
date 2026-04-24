#include "enemies/Enemy.hpp"

Enemy::Enemy(sf::Vector2f pos,
             float spriteW, float spriteH,
             float hitW,    float hitH,
             float speed,
             int   hitsToEncase)
    : Entity(pos)
    , m_sprite(m_texture)
    , m_spriteWidth(spriteW)
    , m_spriteHeight(spriteH)
    , m_hitOffsetX((spriteW - hitW) / 2.f)
    , m_hitOffsetY(spriteH - hitH)
    , m_speed(speed)
    , m_gravity(800.f)
    , m_jumpForce(-430.f)   // tuned weaker than Player's -450 — enemies shouldn't leap as high
    , m_facingRight(true)
    , m_onGround(false)
    , m_state(State::Alive)
    , m_hitsToEncase(hitsToEncase)
    , m_hitsTaken(0)
{
    hitBox.size     = { hitW, hitH };
    hitBox.position = { pos.x + m_hitOffsetX, pos.y + m_hitOffsetY };
}

void Enemy::applyGravity(float dt) {
    velocity.y += m_gravity * dt;
}

void Enemy::syncHitBox() {
    hitBox.position = { position.x + m_hitOffsetX,
                        position.y + m_hitOffsetY };
}

void Enemy::syncSpritePosition() {
    auto texSize = m_texture.getSize();
    if (texSize.x == 0 || texSize.y == 0) return;

    float absScaleX = m_spriteWidth  / static_cast<float>(texSize.x);
    float absScaleY = m_spriteHeight / static_cast<float>(texSize.y);

    m_sprite.setOrigin({ static_cast<float>(texSize.x) / 2.f, 0.f });

    if (m_facingRight) {
        m_sprite.setScale({ absScaleX, absScaleY });
    } else {
        m_sprite.setScale({ -absScaleX, absScaleY });
    }

    m_sprite.setPosition({ position.x + m_spriteWidth / 2.f, position.y });
}

void Enemy::jump() {
    // Only grounded enemies can jump — same guard Player uses.
    if (!m_onGround) return;
    velocity.y = m_jumpForce;
    m_onGround = false;
}

void Enemy::update(float dt) {
    applyGravity(dt);
    updateAI(dt);
    position += velocity * dt;
    syncHitBox();
    syncSpritePosition();
}

void Enemy::draw(sf::RenderWindow& window) {
    window.draw(m_sprite);
}

void Enemy::setPosition(sf::Vector2f pos) {
    position = pos;
    syncHitBox();
    syncSpritePosition();
}

void Enemy::takeSnowballHit() {
    ++m_hitsTaken;
    if (m_hitsTaken >= m_hitsToEncase && m_state == State::Alive) {
        m_state = State::Snowballed;
        velocity.x = 0.f;
    }
}