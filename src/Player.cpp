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

    struct AnimInfo {
        const char* file;
        int         frameCount;
        float       frameDuration;
    };

    struct StateFrames {
        AnimInfo frames[8];
    };

    const StateFrames ANIM_TABLE[3][4] = {
        // ---- Blue player (character 0) ----
        {
            { {{ "assets/sprites/player_blue_idle.png",         1, 0.18f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_blue_walk_frame1.png",  1, 0.33f },
               { "assets/sprites/player_blue_walk_frame2.png",  1, 0.33f },
               { "assets/sprites/player_blue_walk_frame3.png",  1, 0.33f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_blue_jump.png",         1, 0.15f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_blue_throw_frame1.png", 1, 0.15f },
               { "assets/sprites/player_blue_throw_frame2.png", 1, 0.15f },
               { nullptr, 0, 0.f }} },
        },
        // ---- Red player (character 1) ----
        {
            { {{ "assets/sprites/player_red_idle.png",          1, 0.18f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_red_walk_frame1.png",   1, 0.33f },
               { "assets/sprites/player_red_walk_frame2.png",   1, 0.33f },
               { "assets/sprites/player_red_walk_frame3.png",   1, 0.33f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_red_jump.png",          1, 0.15f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_red_throw_frame1.png",  1, 0.15f },
               { "assets/sprites/player_red_throw_frame2.png",  1, 0.15f },
               { nullptr, 0, 0.f }} },
        },
        // ---- Modi player (character 2) ----
        {
            { {{ "assets/sprites/player_modi_idle.png",          1, 0.18f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_modi_walk_frame1.png",   1, 0.33f },
               { "assets/sprites/player_modi_walk_frame2.png",   1, 0.33f },
               { "assets/sprites/player_modi_walk_frame3.png",   1, 0.33f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_modi_jump.png",          1, 0.15f },
               { nullptr, 0, 0.f }} },
            { {{ "assets/sprites/player_modi_throw_frame1.png",  1, 0.15f },
               { "assets/sprites/player_modi_throw_frame2.png",  1, 0.15f },
               { nullptr, 0, 0.f }} },
        },
    };

}

Player::Player(sf::Vector2f pos, int characterIndex)
    : Entity(pos)
    , m_currentAnim(AnimState::Idle)
    , m_currentFrame(0)
    , m_animTimer(0.f)
    , m_sprite(m_animations[0].texture)
    , m_lives(2)
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
    loadAnimations(characterIndex);
    hitBox.size     = { HITBOX_WIDTH,  HITBOX_HEIGHT };
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
}

void Player::loadAnimations(int characterIndex) {
    int idx = (characterIndex >= 0 && characterIndex < 3) ? characterIndex : 0;

    for (int s = 0; s < 4; ++s) {
        Animation& anim      = m_animations[s];
        const StateFrames& sf_ = ANIM_TABLE[idx][s];

        int loaded = 0;
        for (int f = 0; f < 8; ++f) {
            const AnimInfo& info = sf_.frames[f];
            if (!info.file) break;

            if (anim.frameTextures[f].loadFromFile(info.file)) {
                anim.frameDurations[f] = info.frameDuration;
                ++loaded;
                std::cout << "[Player] Loaded anim[" << s << "] frame " << f
                          << ": " << info.file << "\n";
            } else {
                std::cerr << "[Player] Missing anim[" << s << "] frame " << f
                          << ": " << info.file << " — stopping frame load for this state\n";
                break;
            }
        }

        if (loaded > 0) {
            anim.frameCount    = loaded;
            anim.frameDuration = anim.frameDurations[0];
            anim.frameW        = static_cast<int>(anim.frameTextures[0].getSize().x);
            anim.frameH        = static_cast<int>(anim.frameTextures[0].getSize().y);
            anim.activeTexture = &anim.frameTextures[0];
            anim.loaded        = true;
        } else {
            anim.frameCount    = 0;
            anim.frameDuration = 0.15f;
            anim.frameW        = 0;
            anim.frameH        = 0;
            anim.activeTexture = nullptr;
            anim.loaded        = false;
            if (s > 0) {
                std::cerr << "[Player] State " << s
                          << " completely missing — will use idle fallback\n";
            }
        }
    }

    // Fallback: wire missing states to idle
    if (m_animations[0].loaded) {
        for (int s = 1; s < 4; ++s) {
            if (!m_animations[s].loaded) {
                m_animations[s].activeTexture = m_animations[0].activeTexture;
                m_animations[s].frameCount    = m_animations[0].frameCount;
                m_animations[s].frameDuration = m_animations[0].frameDuration;
                m_animations[s].frameW        = m_animations[0].frameW;
                m_animations[s].frameH        = m_animations[0].frameH;
                m_animations[s].loaded        = true;
            }
        }
    }

    // Prime the sprite with the first idle frame
    if (m_animations[0].loaded && m_animations[0].activeTexture) {
        m_sprite.setTexture(*m_animations[0].activeTexture, true);
    }

    applySpriteTransform();
}

void Player::updateAnimation(float dt) {
    // Decide desired state
    AnimState desired;
    if (m_wantsToThrow || m_throwCooldown > (m_throwInterval - 0.10f)) {
        desired = AnimState::Throw;
    } else if (!onGround) {
        desired = AnimState::Jump;
    } else if (velocity.x != 0.f) {
        desired = AnimState::Walk;
    } else {
        desired = AnimState::Idle;
    }

    // Reset frame when state changes
    if (desired != m_currentAnim) {
        m_currentAnim  = desired;
        m_currentFrame = 0;
        m_animTimer    = 0.f;
    }

    Animation& anim = m_animations[static_cast<int>(m_currentAnim)];
    if (!anim.loaded || anim.frameCount <= 0) return;

    // Clamp frame index
    if (m_currentFrame >= anim.frameCount) m_currentFrame = 0;

    // Use per-frame duration
    float frameDur = (m_currentFrame < 8) ? anim.frameDurations[m_currentFrame]
                                           : anim.frameDuration;
    if (frameDur <= 0.f) frameDur = 0.15f;

    // Advance timer
    m_animTimer += dt;
    if (m_animTimer >= frameDur) {
        m_animTimer -= frameDur;
        m_currentFrame = (m_currentFrame + 1) % anim.frameCount;
    }

    // Point activeTexture at the current frame
    if (m_currentFrame < 8) {
        anim.activeTexture = &anim.frameTextures[m_currentFrame];
    }
    if (anim.activeTexture) {
        m_sprite.setTexture(*anim.activeTexture, true);
    }
}

void Player::applySpriteTransform() {
    Animation& anim = m_animations[static_cast<int>(m_currentAnim)];
    if (!anim.loaded || anim.frameW <= 0 || anim.frameH <= 0) return;

    float scaleX = PLAYER_WIDTH  / static_cast<float>(anim.frameW);
    float scaleY = PLAYER_HEIGHT / static_cast<float>(anim.frameH);

    m_sprite.setOrigin({ static_cast<float>(anim.frameW) / 2.f, 0.f });

    // Sprites face LEFT by default → negative scaleX flips to face right
    if (m_facingRight)
        m_sprite.setScale({ -scaleX,  scaleY });
    else
        m_sprite.setScale({  scaleX,  scaleY });

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

    // K key toggles auto-attack
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

    if (m_invincibleTimer > 0.f) {
        m_invincibleTimer -= dt;
        int phase = static_cast<int>(m_invincibleTimer * 10.f);
        m_blinkVisible = (phase % 2 == 0);
        if (m_invincibleTimer <= 0.f) {
            m_invincibleTimer = 0.f;
            m_blinkVisible    = true;
        }
    }

    handleInput();
    applyGravity(dt);
    updateAnimation(dt);

    position       += velocity * dt;
    hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };

    if (m_balloonMode) {
        const float TOP_BOUNDARY    = 15.f;
        const float BOTTOM_BOUNDARY = 585.f;
        const float LEFT_BOUNDARY   = 30.f;
        const float RIGHT_BOUNDARY  = 770.f;

        if (position.y < TOP_BOUNDARY) {
            position.y = TOP_BOUNDARY;
            velocity.y = -velocity.y;
        }
        if (position.y + PLAYER_HEIGHT > BOTTOM_BOUNDARY) {
            position.y = BOTTOM_BOUNDARY - PLAYER_HEIGHT;
            velocity.y = -velocity.y;
        }
        if (position.x < LEFT_BOUNDARY) {
            position.x = LEFT_BOUNDARY;
            velocity.x = -velocity.x;
        }
        if (position.x + PLAYER_WIDTH > RIGHT_BOUNDARY) {
            position.x = RIGHT_BOUNDARY - PLAYER_WIDTH;
            velocity.x = -velocity.x;
        }

        hitBox.position = { position.x + HITBOX_OFFSET_X, position.y + HITBOX_OFFSET_Y };
    }

    applySpriteTransform();
}

void Player::draw(sf::RenderWindow& window) {
    if (m_invincibleTimer > 0.f && !m_blinkVisible) return;
    window.draw(m_sprite);
}

void Player::setPosition(sf::Vector2f pos) {
    position        = pos;
    hitBox.position = { pos.x + HITBOX_OFFSET_X, pos.y + HITBOX_OFFSET_Y };
    applySpriteTransform();
}

void Player::loseLife() {
    if (m_invincibleTimer > 0.f) return;
    if (m_lives <= 0) return;
    --m_lives;
    m_invincibleTimer = 3.0f;
    m_blinkVisible    = true;
}

void Player::respawn(sf::Vector2f spawnPos) {
    setPosition(spawnPos);
    velocity       = { 0.f, 0.f };
    onGround       = false;
    m_facingRight  = true;
    m_currentAnim  = AnimState::Idle;
    m_currentFrame = 0;
    m_animTimer    = 0.f;
}