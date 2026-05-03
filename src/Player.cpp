#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

namespace {
    const float PLAYER_WIDTH  = 56.f;
    const float PLAYER_HEIGHT = 68.f;
    const float HITBOX_WIDTH  = 24.f;
    const float HITBOX_HEIGHT = 57.f;
    const float HITBOX_OFFSET_X = (PLAYER_WIDTH  - HITBOX_WIDTH)  / 2.f;
    const float HITBOX_OFFSET_Y =  PLAYER_HEIGHT - HITBOX_HEIGHT;

    // Animation timing
    const float WALK_FRAME_TIME  = 0.33f;   // each walk frame = 0.33s, full cycle = 1s
    const float THROW_FRAME_TIME = 0.15f;   // each throw frame = 0.15s, full anim = 0.3s
}

Player::Player(sf::Vector2f pos)
    : Entity(pos)
    , m_idleLoaded(false)
    , m_walkLoaded(false)
    , m_jumpLoaded(false)
    , m_throwLoaded(false)
    , m_sprite(m_idleTexture)
    , m_walkFrame(0)
    , m_walkTimer(0.f)
    , m_throwFrame(0)
    , m_throwTimer(0.f)
    , m_isThrowing(false)
    , m_lives(10)
    , m_invincibleTimer(0.f)
    , m_blinkVisible(true)
    , m_throwCooldown(0.f)
    , m_throwInterval(0.18f)
    , m_wantsToThrow(false)
    , m_autoAttack(false)
    , m_autoAttackTimer(0.f)
    , speed(200.f)
    , jumpForce(-450.f)
    , gravity(800.f)
    , onGround(false)
    , m_facingRight(true)
    , m_balloonMode(false)
    , m_balloonGravity(-50.f)
{
    loadAnimations();

    if (m_idleLoaded) {
        m_sprite.setTexture(m_idleTexture, true);
    }

    m_sprite.setPosition(pos);
    hitBox.size     = { HITBOX_WIDTH, HITBOX_HEIGHT };
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
}

void Player::loadAnimations() {
    // === IDLE (1 frame) ===
    m_idleLoaded = m_idleTexture.loadFromFile("assets/sprites/player_blue_idle.png");
    if (!m_idleLoaded)
        std::cerr << "[Player] Failed to load player_blue_idle.png\n";

    // === WALK (3 frames) ===
    m_walkLoaded = true;
    for (int i = 0; i < 3; ++i) {
        std::string path = "assets/sprites/player_blue_walk_frame" + std::to_string(i + 1) + ".png";
        if (!m_walkTextures[i].loadFromFile(path)) {
            std::cerr << "[Player] Failed to load " << path << "\n";
            m_walkLoaded = false;
            break;
        }
    }

    // === JUMP (1 frame — used for both jump and fall) ===
    m_jumpLoaded = m_jumpTexture.loadFromFile("assets/sprites/player_blue_jump.png");
    if (!m_jumpLoaded)
        std::cerr << "[Player] Failed to load player_blue_jump.png\n";

    // === THROW (2 frames) ===
    m_throwLoaded = true;
    for (int i = 0; i < 2; ++i) {
        std::string path = "assets/sprites/player_blue_throw_frame" + std::to_string(i + 1) + ".png";
        if (!m_throwTextures[i].loadFromFile(path)) {
            std::cerr << "[Player] Failed to load " << path << "\n";
            m_throwLoaded = false;
            break;
        }
    }
}

// ---------------------------------------------------------------
// updateAnimation — pick the right texture, swap it onto the sprite.
// Priority: throw > airborne > walk > idle
// Movement is NEVER blocked by any animation.
// ---------------------------------------------------------------
void Player::updateAnimation(float dt) {

    // --- THROW ANIMATION (overlays on top of other states) ---
    if (m_isThrowing && m_throwLoaded) {
        m_throwTimer += dt;
        if (m_throwTimer >= THROW_FRAME_TIME) {
            m_throwTimer -= THROW_FRAME_TIME;
            m_throwFrame++;
            if (m_throwFrame >= 2) {
                m_isThrowing = false;   // throw anim finished
                m_throwFrame = 0;
                m_throwTimer = 0.f;
                // fall through to pick walk/idle/jump below
            }
        }
        if (m_isThrowing) {
            m_sprite.setTexture(m_throwTextures[m_throwFrame], true);
            return;
        }
    }

    // --- AIRBORNE (jump / fall — same frame) ---
    if (!onGround && m_jumpLoaded) {
        m_sprite.setTexture(m_jumpTexture, true);
        // Reset walk frame so walk starts clean on landing
        m_walkFrame = 0;
        m_walkTimer = 0.f;
        return;
    }

    // --- WALKING ---
    if (velocity.x != 0.f && m_walkLoaded && onGround) {
        m_walkTimer += dt;
        if (m_walkTimer >= WALK_FRAME_TIME) {
            m_walkTimer -= WALK_FRAME_TIME;
            m_walkFrame = (m_walkFrame + 1) % 3;
        }
        m_sprite.setTexture(m_walkTextures[m_walkFrame], true);
        return;
    }

    // --- IDLE (standing still on ground) ---
    if (m_idleLoaded) {
        m_sprite.setTexture(m_idleTexture, true);
    }
    // Reset walk so it starts from frame 0 next time
    m_walkFrame = 0;
    m_walkTimer = 0.f;
}

// ---------------------------------------------------------------
// applySpriteTransform — scale, flip, and position the sprite.
// Sprites face LEFT by default. Flip for right.
// ---------------------------------------------------------------
void Player::applySpriteTransform() {
    const sf::Texture& tex = m_sprite.getTexture();
    auto texSize = tex.getSize();
    if (texSize.x == 0 || texSize.y == 0) return;

    float absScaleX = PLAYER_WIDTH  / static_cast<float>(texSize.x);
    float absScaleY = PLAYER_HEIGHT / static_cast<float>(texSize.y);

    m_sprite.setOrigin({ static_cast<float>(texSize.x) / 2.f, 0.f });

    // Default facing LEFT → flip X when facing right
    if (m_facingRight)
        m_sprite.setScale({-absScaleX, absScaleY});
    else
        m_sprite.setScale({ absScaleX, absScaleY});

    m_sprite.setPosition({ position.x + PLAYER_WIDTH / 2.f, position.y });
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

    // Jump
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        && onGround) {
        velocity.y = jumpForce;
        onGround = false;
    }

    // Throw (movement continues normally)
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
       || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        && m_throwCooldown <= 0.f) {
        m_wantsToThrow = true;
    }

    // K key toggles auto-attack (edge-detected via static bool)
    {
        static bool s_kWasPressed = false;
        bool kNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K);
        if (kNow && !s_kWasPressed) {
            m_autoAttack = !m_autoAttack;
            m_autoAttackTimer = 0.f;
            std::cout << "[Player] Auto-attack " << (m_autoAttack ? "ON" : "OFF") << "\n";
        }
        s_kWasPressed = kNow;
    }

    // Auto-attack fires at the same interval as manual throw
    if (m_autoAttack && m_throwCooldown <= 0.f) {
        m_wantsToThrow = true;
    }
}

void Player::applyGravity(float dt) {
    if (m_balloonMode)
        velocity.y += m_balloonGravity * dt;
    else
        velocity.y += gravity * dt;
}

void Player::setSpeedMultiplier(float multiplier) {
    const float BASE_SPEED = 200.f;
    speed = BASE_SPEED * multiplier;
}

void Player::update(float dt) {
    if (m_throwCooldown > 0.f) m_throwCooldown -= dt;

    // Invincibility blink
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

    // Balloon mode boundary reflection
    if (m_balloonMode) {
        const float TOP = 15.f, BOTTOM = 585.f, LEFT = 30.f, RIGHT = 770.f;
        if (position.y < TOP)                       { position.y = TOP;                     velocity.y = -velocity.y; }
        if (position.y + PLAYER_HEIGHT > BOTTOM)    { position.y = BOTTOM - PLAYER_HEIGHT;  velocity.y = -velocity.y; }
        if (position.x < LEFT)                      { position.x = LEFT;                    velocity.x = -velocity.x; }
        if (position.x + PLAYER_WIDTH > RIGHT)      { position.x = RIGHT - PLAYER_WIDTH;    velocity.x = -velocity.x; }
        hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };
    }

    updateAnimation(dt);
    applySpriteTransform();
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
    velocity = { 0.f, 0.f };
    onGround = false;
    m_facingRight = true;
    m_isThrowing = false;
}