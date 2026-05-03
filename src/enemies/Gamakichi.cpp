#include "enemies/Gamakichi.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cmath>

// Candidate bomb targets — (x centre, y surface) on each platform
const sf::Vector2f Gamakichi::s_targets[NUM_TARGETS] = {
    {160.f, 240.f},   // upper-left platform
    {270.f, 240.f},
    {560.f, 240.f},   // upper-right platform
    {670.f, 240.f},
    {280.f, 349.f},   // mid-centre platform
    {490.f, 349.f},
};

// ---------------------------------------------------------------
Gamakichi::Gamakichi(sf::Vector2f pos)
    : Entity(pos)
    , m_closedLoaded(false), m_openLoaded(false)
    , m_sprite(m_closedTex)
    , m_dangerLoaded(false), m_dangerSprite(m_dangerTex)
    , m_bossState(BossState::Idle)
    , m_stateTimer(IDLE_DURATION)
    , m_hitsRemaining(150), m_maxHits(150)
    , m_sinkOffset(0.f)
    , m_bombCount(0)
    , m_rewardPending(false)
    , m_dangerVisible(false)
{
    for (int i = 0; i < MAX_BOMBS; ++i) m_bombs[i] = nullptr;
    for (int i = 0; i < MAX_DANGER; ++i) m_dangerPos[i] = {0.f,0.f};

    auto tryLoad = [](sf::Texture& t, bool& f, const char* p) {
        if (std::FILE* fp = std::fopen(p,"rb")) { std::fclose(fp); f = t.loadFromFile(p); }
        else { std::cerr << "[Gamakichi] missing: " << p << "\n"; f = false; }
    };
    tryLoad(m_closedTex, m_closedLoaded, "assets/sprites/gama_mouth_closed.png");
    tryLoad(m_openTex,   m_openLoaded,   "assets/sprites/gama_open_mouth_ready_to_attack.png");
    tryLoad(m_dangerTex, m_dangerLoaded,
            "assets/sprites/it_will_indicate_the_danger_zone_before_2_seconds_of_explosion.png");

    if (m_closedLoaded) m_sprite.setTexture(m_closedTex, true);

    // Hitbox — spans most of lower-right area
    hitBox.size     = {200.f, 220.f};
    hitBox.position = pos;

    syncSprite();
}

Gamakichi::~Gamakichi() {
    for (int i = 0; i < m_bombCount; ++i) { delete m_bombs[i]; m_bombs[i] = nullptr; }
}

// ---------------------------------------------------------------
void Gamakichi::syncSprite() {
    sf::Vector2u sz = m_sprite.getTexture().getSize();
    if (sz.x > 0 && sz.y > 0)
        m_sprite.setScale({200.f/(float)sz.x, 220.f/(float)sz.y});
    m_sprite.setPosition({position.x, position.y + m_sinkOffset});
}

// ---------------------------------------------------------------
void Gamakichi::pickDangerZones() {
    // Pick 2 distinct random targets from s_targets
    int a = std::rand() % NUM_TARGETS;
    int b;
    do { b = std::rand() % NUM_TARGETS; } while (b == a);
    m_dangerPos[0] = s_targets[a];
    m_dangerPos[1] = s_targets[b];
    m_dangerVisible = true;
}

void Gamakichi::fireBombs() {
    // Clean old bombs
    for (int i = 0; i < m_bombCount; ++i) { delete m_bombs[i]; m_bombs[i] = nullptr; }
    m_bombCount = 0;

    // Mouth position (left side of Gamakichi sprite)
    sf::Vector2f mouth = {position.x + 20.f, position.y + 80.f};

    for (int i = 0; i < MAX_DANGER && m_bombCount < MAX_BOMBS; ++i) {
        m_bombs[m_bombCount++] = new GamaBomb(mouth, m_dangerPos[i]);
    }
    m_dangerVisible = false;
}

void Gamakichi::updateBombs(float dt) {
    for (int i = 0; i < m_bombCount; ++i)
        if (m_bombs[i]) m_bombs[i]->update(dt);
}

// ---------------------------------------------------------------
Gamakichi::RewardRequest Gamakichi::getAndClearRewardPending() {
    RewardRequest r; r.pending = m_rewardPending;
    m_rewardPending = false; return r;
}

// ---------------------------------------------------------------
void Gamakichi::takeSnowballHit() {
    if (m_bossState == BossState::Dying || m_bossState == BossState::Dead) return;
    --m_hitsRemaining;
    std::cout << "[Gamakichi] Hit! HP remaining: " << m_hitsRemaining << "\n";
    if (m_hitsRemaining <= 0) {
        m_hitsRemaining = 0;
        m_bossState     = BossState::Dying;
        m_stateTimer    = 0.f;
        m_sinkOffset    = 0.f;
        m_dangerVisible = false;
        m_rewardPending = true;
        std::cout << "[Gamakichi] Defeated!\n";
    }
}

// ---------------------------------------------------------------
void Gamakichi::update(float dt) {
    if (m_bossState == BossState::Dead) return;

    if (m_bossState == BossState::Dying) {
        m_stateTimer += dt;
        // Sink downward off screen
        m_sinkOffset = (m_stateTimer / SINK_DURATION) * 300.f;
        if (m_stateTimer >= SINK_DURATION) {
            m_bossState = BossState::Dead;
            alive = false;
        }
        syncSprite();
        return;
    }

    m_stateTimer += dt;

    switch (m_bossState) {
        case BossState::Idle:
            if (m_stateTimer >= IDLE_DURATION) {
                m_bossState  = BossState::OpenMouth;
                m_stateTimer = 0.f;
                // Switch to open-mouth sprite
                if (m_openLoaded) m_sprite.setTexture(m_openTex, true);
                syncSprite();
                pickDangerZones();
                std::cout << "[Gamakichi] Opening mouth — danger zones set\n";
            }
            break;

        case BossState::OpenMouth:
            if (m_stateTimer >= OPEN_DURATION) {
                m_bossState  = BossState::Danger;
                m_stateTimer = 0.f;
            }
            break;

        case BossState::Danger:
            if (m_stateTimer >= DANGER_DURATION) {
                m_bossState  = BossState::Firing;
                m_stateTimer = 0.f;
                fireBombs();
                std::cout << "[Gamakichi] FIRE!\n";
            }
            break;

        case BossState::Firing:
            updateBombs(dt);
            // Wait until all bombs are dead
            {
                bool allDone = true;
                for (int i = 0; i < m_bombCount; ++i)
                    if (m_bombs[i] && !m_bombs[i]->isDead()) { allDone = false; break; }
                if (allDone) {
                    // Return to idle, switch back to closed sprite
                    m_bossState  = BossState::Idle;
                    m_stateTimer = 0.f;
                    if (m_closedLoaded) m_sprite.setTexture(m_closedTex, true);
                    syncSprite();
                }
            }
            break;

        default: break;
    }

    // Always update bombs if they exist
    if (m_bossState != BossState::Firing) updateBombs(dt);
}

// ---------------------------------------------------------------
void Gamakichi::draw(sf::RenderWindow& window) {
    if (m_bossState == BossState::Dead) return;

    // Draw danger zone markers
    if (m_dangerVisible && m_dangerLoaded) {
        sf::Vector2u ds = m_dangerTex.getSize();
        for (int i = 0; i < MAX_DANGER; ++i) {
            m_dangerSprite.setTexture(m_dangerTex, true);
            if (ds.x > 0 && ds.y > 0)
                m_dangerSprite.setScale({60.f/ds.x, 40.f/ds.y});
            m_dangerSprite.setPosition({m_dangerPos[i].x - 30.f, m_dangerPos[i].y - 40.f});
            window.draw(m_dangerSprite);
        }
    } else if (m_dangerVisible) {
        // Fallback: red X
        for (int i = 0; i < MAX_DANGER; ++i) {
            sf::RectangleShape r({60.f, 8.f});
            r.setFillColor(sf::Color(255, 30, 30, 200));
            r.setPosition({m_dangerPos[i].x - 30.f, m_dangerPos[i].y - 20.f});
            window.draw(r);
        }
    }

    // Draw bombs
    drawBombs(window);

    // Draw Gamakichi sprite
    window.draw(m_sprite);
}

void Gamakichi::drawBombs(sf::RenderWindow& window) {
    for (int i = 0; i < m_bombCount; ++i)
        if (m_bombs[i]) m_bombs[i]->draw(window);
}

void Gamakichi::drawHitBoxDebug(sf::RenderWindow& window, sf::Color c) {
    sf::RectangleShape r({hitBox.size.x, hitBox.size.y});
    r.setPosition({hitBox.position.x, hitBox.position.y});
    r.setFillColor(sf::Color::Transparent);
    r.setOutlineColor(c);
    r.setOutlineThickness(2.f);
    window.draw(r);
}