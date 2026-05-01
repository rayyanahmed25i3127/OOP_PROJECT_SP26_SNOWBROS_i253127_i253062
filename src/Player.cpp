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

<<<<<<< HEAD
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
=======
    // Animation timing
    const float WALK_FRAME_TIME  = 0.33f;   // each walk frame = 0.33s, full cycle = 1s
    const float THROW_FRAME_TIME = 0.15f;   // each throw frame = 0.15s, full anim = 0.3s
}

Player::Player(sf::Vector2f pos)
>>>>>>> origin/main
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
<<<<<<< HEAD
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
=======
    , m_balloonMode(false)
    , m_balloonGravity(-50.f)
{
    loadAnimations();

    if (m_idleLoaded) {
        m_sprite.setTexture(m_idleTexture, true);
>>>>>>> origin/main
    }

    m_sprite.setTexture(m_texture, true);
    auto texSize = m_texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        m_sprite.setScale({ PLAYER_WIDTH  / static_cast<float>(texSize.x),
                            PLAYER_HEIGHT / static_cast<float>(texSize.y) });
    }
    m_sprite.setPosition(pos);
<<<<<<< HEAD

=======
>>>>>>> origin/main
    hitBox.size     = { HITBOX_WIDTH, HITBOX_HEIGHT };
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
}

<<<<<<< HEAD
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
=======
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
>>>>>>> origin/main
}

void Player::handleInput() {
    velocity.x = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        velocity.x = -speed;  m_facingRight = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        velocity.x =  speed;  m_facingRight = true;
    }
<<<<<<< HEAD
=======

    // Jump
>>>>>>> origin/main
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
      || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        && onGround) {
        velocity.y = jumpForce;
        onGround   = false;
    }
<<<<<<< HEAD
=======

    // Throw (movement continues normally)
>>>>>>> origin/main
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
      || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J))
        && m_throwCooldown <= 0.f) {
        m_wantsToThrow = true;
    }
}

void Player::applyGravity(float dt) {
<<<<<<< HEAD
    velocity.y += (m_balloonMode ? m_balloonGravity : gravity) * dt;
}

void Player::setSpeedMultiplier(float multiplier) {
    speed = 200.f * multiplier;
=======
    if (m_balloonMode)
        velocity.y += m_balloonGravity * dt;
    else
        velocity.y += gravity * dt;
}

void Player::setSpeedMultiplier(float multiplier) {
    const float BASE_SPEED = 200.f;
    speed = BASE_SPEED * multiplier;
>>>>>>> origin/main
}

void Player::update(float dt) {
    if (m_throwCooldown > 0.f) m_throwCooldown -= dt;

<<<<<<< HEAD
    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= dt;
        m_blinkVisible = (static_cast<int>(m_invincibleTimer * 10.f) % 2 == 0);
        if (m_invincibleTimer <= 0.f) { m_invincibleTimer = 0.f; m_blinkVisible = true; }
=======
    // Invincibility blink
    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= dt;
        int phase = static_cast<int>(m_invincibleTimer * 10.f);
        m_blinkVisible = (phase % 2 == 0);
        if (m_invincibleTimer <= 0.f) {
            m_invincibleTimer = 0.f;
            m_blinkVisible = true;
        }
>>>>>>> origin/main
    }

    handleInput();
    applyGravity(dt);
    position += velocity * dt;
    hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };

<<<<<<< HEAD
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
=======
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
>>>>>>> origin/main
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
<<<<<<< HEAD
    if (m_invincibleTimer > 0.f || m_lives <= 0) return;
    --m_lives;
    m_invincibleTimer = 3.0f;
    m_blinkVisible    = true;
=======
    if (m_invincibleTimer > 0.f) return;
    if (m_lives <= 0) return;
    --m_lives;
    m_invincibleTimer = 3.0f;
    m_blinkVisible = true;
>>>>>>> origin/main
}

void Player::respawn(sf::Vector2f spawnPos) {
    setPosition(spawnPos);
    velocity      = { 0.f, 0.f };
    onGround      = false;
    m_facingRight = true;
    m_isThrowing = false;
}