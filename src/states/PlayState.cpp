#include "states/PlayState.hpp"
#include "states/StateManager.hpp"
#include "states/PauseState.hpp"
#include "states/GameOverState.hpp"
#include "enemies/Botom.hpp"
#include "audio/AudioManager.hpp"
#include "effects/HitFlash.hpp"
#include "powerups/PowerUp.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>

std::string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[11];
    sprintf(buffer, "%04d-%02d-%02d",
        1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
    return std::string(buffer);
}

void saveScore(const std::string& name, int score, int level) {
    std::ofstream file("leaderboard.txt", std::ios::app);
    if (!file.is_open()) { std::cerr << "Failed to open leaderboard.txt\n"; return; }
    file << name << "," << score << " " << level << " " << getCurrentDate() << "\n";
    file.close();
}

namespace {
    const float WINDOW_WIDTH  = 800.f;
    const float WINDOW_HEIGHT = 600.f;
}

PlayState::PlayState(int characterIndex)
    : m_characterIndex(characterIndex)
    , m_backgroundSprite(m_backgroundTexture)
    , m_backgroundLoaded(false)
    , m_platformTextureLoaded(false)
    , m_platformTopTextureLoaded(false)
    , m_player(nullptr)
    , m_platformCount(0)
    , m_enemyCount(0)
    , m_playerPrevX(0.f)
    , m_playerPrevY(0.f)
    , m_collider(30.f, 770.f)
    , m_showHitboxes(false)
    , m_gameOver(false)
    , m_hudFontLoaded(false)
    , m_heartLoaded(false)
    , m_diamondLoaded(false)
    , m_score(0)
    , m_gems(0)
    , m_currentLevel(1)
    , m_totalLevels(10)
    , m_playerSpawn(100.f, 450.f)
    , m_projectileCount(0)
    , m_hitFlashCount(0)
    , m_powerUpCount(0)
    , m_speedActive(false),     m_speedTimer(0.f)
    , m_balloonActive(false),   m_balloonTimer(0.f)
    , m_snowballPowerActive(false)
    , m_distanceActive(false)
    , m_displayedType(PowerUp::Type::SpeedBoost)
    , m_hasDisplayed(false)
    , m_puIconSpeedLoaded(false)
    , m_puIconSnowballLoaded(false)
    , m_puIconDistanceLoaded(false)
    , m_puIconBalloonLoaded(false)
{
    for (int i = 0; i < MAX_HIT_FLASHES;  ++i) m_hitFlashes[i]  = nullptr;
    for (int i = 0; i < MAX_ENEMIES;      ++i) m_chainCount[i]  = 0;
    for (int i = 0; i < MAX_PROJECTILES;  ++i) m_projectiles[i] = nullptr;
    for (int i = 0; i < MAX_PLATFORMS;    ++i) m_platforms[i]   = nullptr;
    for (int i = 0; i < MAX_ENEMIES;      ++i) {
        m_enemies[i]    = nullptr;
        m_enemyPrevX[i] = 0.f;
        m_enemyPrevY[i] = 0.f;
    }
    for (int i = 0; i < MAX_POWERUPS; ++i) m_powerUps[i] = nullptr;
}

PlayState::~PlayState() {
    delete m_player; m_player = nullptr;
    for (int i = 0; i < m_platformCount;  ++i) { delete m_platforms[i];  m_platforms[i]  = nullptr; }
    for (int i = 0; i < m_enemyCount;     ++i) { delete m_enemies[i];    m_enemies[i]    = nullptr; }
    for (int i = 0; i < m_projectileCount;++i) { delete m_projectiles[i];m_projectiles[i]= nullptr; }
    for (int i = 0; i < m_hitFlashCount;  ++i)   delete m_hitFlashes[i];
    for (int i = 0; i < m_powerUpCount;   ++i) { delete m_powerUps[i];   m_powerUps[i]   = nullptr; }
}

void PlayState::onEnter() {
    m_playerName = m_manager->getCurrentUserName();
    std::cout << "[PlayState] Entering gameplay\n";

    // ── Load background ──────────────────────────────────────────────────
    if (!m_backgroundTexture.loadFromFile("assets/sprites/bg_lvl1.png")) {
        std::cerr << "[PlayState] Could not load bg_lvl1.png\n";
        m_backgroundLoaded = false;
    } else {
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        auto sz = m_backgroundTexture.getSize();
        m_backgroundSprite.setScale({ WINDOW_WIDTH  / static_cast<float>(sz.x),
                                      WINDOW_HEIGHT / static_cast<float>(sz.y) });
        m_backgroundLoaded = true;
    }

    if (!m_platformTexture.loadFromFile("assets/sprites/platform_1.png")) {
        std::cerr << "[PlayState] Could not load platform_1.png\n";
        m_platformTextureLoaded = false;
    } else { m_platformTextureLoaded = true; }

    if (!m_platformTopTexture.loadFromFile("assets/sprites/platform_2.png")) {
        std::cerr << "[PlayState] Could not load platform_2.png\n";
        m_platformTopTextureLoaded = false;
    } else { m_platformTopTextureLoaded = true; }

    // ── Spawn player — MUST happen before applying power-ups ────────────
    m_player = new Player(m_playerSpawn, m_characterIndex);

    // ── Apply power-ups purchased from the shop ──────────────────────────
    // All checks happen AFTER m_player is valid.
    // After applying, reset all flags so items become purchasable again
    // on the next shop visit.
    {
        if (m_manager->isSpeedBought()) {
            m_speedActive = true;
            m_speedTimer  = 30.f;            // 2× the normal 15 s — reward for buying
            m_player->setSpeedMultiplier(1.5f);
            m_displayedType = PowerUp::Type::SpeedBoost;
            m_hasDisplayed  = true;
            std::cout << "[PlayState] Shop power-up applied: Speed Boost\n";
        }

        if (m_manager->isBalloonBought()) {
            m_balloonActive = true;
            m_balloonTimer  = 20.f;          // 2× the normal 10 s
            m_player->setBalloonMode(true);
            m_displayedType = PowerUp::Type::BalloonMode;
            m_hasDisplayed  = true;
            std::cout << "[PlayState] Shop power-up applied: Balloon Mode\n";
        }

        if (m_manager->isSnowballBought()) {
            m_snowballPowerActive = true;
            m_displayedType = PowerUp::Type::SnowballPower;
            m_hasDisplayed  = true;
            std::cout << "[PlayState] Shop power-up applied: Snowball Power\n";
        }

        if (m_manager->isDistanceBought()) {
            m_distanceActive = true;
            m_displayedType  = PowerUp::Type::DistanceIncrease;
            m_hasDisplayed   = true;
            std::cout << "[PlayState] Shop power-up applied: Distance Increase\n";
        }

        if (m_manager->isExtraLifeBought()) {
            // addLife() internally caps at Player::MAX_LIVES (3)
            bool added = m_player->addLife();
            if (!added)
                std::cout << "[PlayState] Extra life bought but player already at max lives\n";
        }

        // ── Reset so the shop items become buyable again next visit ──────
        m_manager->resetBoughtPowerUps();
    }

    // ── HUD assets ───────────────────────────────────────────────────────
    if (!m_hudFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        std::cerr << "[PlayState] Could not load PressStart2P-Regular.ttf\n";
        m_hudFontLoaded = false;
    } else { m_hudFontLoaded = true; }

    if (!m_heartTexture.loadFromFile("assets/sprites/heart.png")) {
        std::cerr << "[PlayState] Could not load heart.png\n";
        m_heartLoaded = false;
    } else { m_heartLoaded = true; }

    if (!m_diamondTexture.loadFromFile("assets/sprites/diamond.png")) {
        std::cerr << "[PlayState] Could not load diamond.png\n";
        m_diamondLoaded = false;
    } else { m_diamondLoaded = true; }

    loadPowerUpIcons();

    // ── Level geometry + enemies ─────────────────────────────────────────
    buildLevel();
    spawnEnemies();

    // Snowball power from shop must also propagate to freshly spawned enemies
    if (m_snowballPowerActive) {
        for (int i = 0; i < m_enemyCount; ++i)
            if (m_enemies[i]) m_enemies[i]->setOneHitEncase(true);
    }

    AudioManager::get().playGameMusic();
}

void PlayState::onExit() {
    std::cout << "[PlayState] Exiting gameplay\n";
    AudioManager::get().playMenuMusic();
}

void PlayState::buildLevel() {
    if (!m_platformTextureLoaded) {
        std::cerr << "[PlayState] Skipping platform build — texture missing\n";
        return;
    }

    const float BORDER_W   = 8.5f;
    const float LEFT_EXTRA = 22.f;
    const float p1H = 78.75f;
    const float p2H = 150.f;
    const float p2W = 630.f;

    {
        const sf::Texture& topTex = m_platformTopTextureLoaded
            ? m_platformTopTexture : m_platformTexture;
        sf::Vector2f p2Size = { p2W, p2H };
        sf::Vector2f p2Pos  = { (WINDOW_WIDTH - p2W) / 2.f, 49.f };
        sf::FloatRect lower; lower.position = { 30.f, 88.f };   lower.size = { p2W - 60.f, 35.f };
        sf::FloatRect upper; upper.position = { 130.f, 45.5f }; upper.size = { p2W - 265.f, 35.f };
        sf::FloatRect boxes[2] = { lower, upper };
        bool solids[2]         = { false, true };
        m_platforms[m_platformCount++] = new Platform(topTex, p2Size, p2Pos, boxes, 2, solids);
    }
    { float w = 300.f;
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 220.f});
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 220.f}); }
    { float w = 390.f;
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {(WINDOW_WIDTH - w) / 2.f, 329.f}); }
    { float w = 300.f;
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {BORDER_W + LEFT_EXTRA, 437.f});
      m_platforms[m_platformCount++] = new Platform(m_platformTexture, {w, p1H}, {WINDOW_WIDTH - BORDER_W - w, 437.f}); }
}

void PlayState::spawnEnemies() {
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(140.f, 400.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(600.f, 400.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(300.f, 290.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(150.f, 180.f));
    m_enemies[m_enemyCount++] = new Botom(sf::Vector2f(620.f, 180.f));
    std::cout << "[PlayState] Spawned " << m_enemyCount << " Botoms\n";
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape && !m_gameOver)
            m_manager->pushState(new PauseState());
        else if (keyEvent->code == sf::Keyboard::Key::F1
              || keyEvent->code == sf::Keyboard::Key::H) {
            m_showHitboxes = !m_showHitboxes;
            std::cout << "[PlayState] Hitboxes " << (m_showHitboxes ? "ON" : "OFF") << "\n";
        }
    }
}

void PlayState::update(float dt) {
    if (m_player) {
        m_playerPrevX = m_player->getPosition().x;
        m_playerPrevY = m_player->getPosition().y;
        m_player->update(dt);
        m_collider.resolve(*m_player, m_platforms, m_platformCount,
                           m_playerPrevX, m_playerPrevY);
    }

    for (int i = 0; i < m_enemyCount; ++i) {
        if (!m_enemies[i]) continue;
        m_enemyPrevX[i] = m_enemies[i]->getPosition().x;
        m_enemyPrevY[i] = m_enemies[i]->getPosition().y;
        m_enemies[i]->update(dt);
        if (m_enemies[i]->getState() != Enemy::State::Rolling)
            m_collider.resolve(*m_enemies[i], m_platforms, m_platformCount,
                               m_enemyPrevX[i], m_enemyPrevY[i]);
    }

    if (m_player && m_player->wantsToThrow() && m_projectileCount < MAX_PROJECTILES) {
        sf::FloatRect pHit = m_player->getHitBox();
        float spawnY = pHit.position.y + pHit.size.y * 0.3f;
        float spawnX = m_player->isFacingRight()
            ? pHit.position.x + pHit.size.x + 2.f
            : pHit.position.x - 16.f - 2.f;
        AttackBall* newBall = new AttackBall({spawnX, spawnY}, m_player->isFacingRight());
        if (m_distanceActive) newBall->setMaxRangeMode(true);
        m_projectiles[m_projectileCount++] = newBall;
        m_player->consumeThrowRequest();
    }

    for (int i = 0; i < m_projectileCount; ++i)
        if (m_projectiles[i]) m_projectiles[i]->update(dt);

    for (int i = 0; i < m_projectileCount; ++i) {
        if (!m_projectiles[i] || !m_projectiles[i]->isAlive()) continue;
        sf::FloatRect pHit = m_projectiles[i]->getHitBox();
        float pL = pHit.position.x, pR = pL + pHit.size.x;
        float pT = pHit.position.y, pB = pT + pHit.size.y;
        for (int e = 0; e < m_enemyCount; ++e) {
            if (!m_enemies[e]) continue;
            Enemy::State s = m_enemies[e]->getState();
            if (s != Enemy::State::Alive && s != Enemy::State::PartialEncase &&
                s != Enemy::State::Escaping75 && s != Enemy::State::Escaping50 &&
                s != Enemy::State::Escaping25) continue;
            sf::FloatRect eHit = m_enemies[e]->getHitBox();
            float eL = eHit.position.x, eR = eL + eHit.size.x;
            float eT = eHit.position.y, eB = eT + eHit.size.y;
            if (!((pR>eL)&&(pL<eR)&&(pB>eT)&&(pT<eB))) continue;
            m_enemies[e]->takeAttackHit();
            m_projectiles[i]->setAlive(false);
            if (m_hitFlashCount < MAX_HIT_FLASHES) {
                sf::Vector2f fp{ (pL+pR)*0.5f-6.f, (pT+pB)*0.5f-8.f };
                m_hitFlashes[m_hitFlashCount++] = new HitFlash(fp);
            }
            break;
        }
    }

    { int w=0;
      for (int r=0; r<m_projectileCount; ++r) {
        if (m_projectiles[r]&&m_projectiles[r]->isAlive()) m_projectiles[w++]=m_projectiles[r];
        else { delete m_projectiles[r]; m_projectiles[r]=nullptr; }
      } m_projectileCount=w; }

    for (int i=0;i<m_hitFlashCount;++i) if(m_hitFlashes[i]) m_hitFlashes[i]->update(dt);
    { int w=0;
      for (int r=0;r<m_hitFlashCount;++r) {
        if(m_hitFlashes[r]&&m_hitFlashes[r]->isAlive()) m_hitFlashes[w++]=m_hitFlashes[r];
        else { delete m_hitFlashes[r]; m_hitFlashes[r]=nullptr; }
      } m_hitFlashCount=w; }

    if (m_player) {
        sf::FloatRect pHit = m_player->getHitBox();
        float pL=pHit.position.x, pR=pL+pHit.size.x, pT=pHit.position.y, pB=pT+pHit.size.y;
        for (int e=0;e<m_enemyCount;++e) {
            if (!m_enemies[e]||m_enemies[e]->getState()!=Enemy::State::Snowballed) continue;
            sf::FloatRect eHit=m_enemies[e]->getHitBox();
            float eL=eHit.position.x,eR=eL+eHit.size.x,eT=eHit.position.y,eB=eT+eHit.size.y;
            if(!((pR>eL)&&(pL<eR)&&(pB>eT)&&(pT<eB))) continue;
            m_enemies[e]->kickIntoRoll(m_player->isFacingRight());
            int ks=randomScore(100,500); m_score+=ks; m_chainCount[e]=1;
            std::cout<<"[PlayState] Kicked Botom into roll. +"<<ks<<". Score:"<<m_score<<"\n";
            break;
        }
    }

    for (int r=0;r<m_enemyCount;++r) {
        if (!m_enemies[r]||m_enemies[r]->getState()!=Enemy::State::Rolling) continue;
        sf::FloatRect rHit=m_enemies[r]->getHitBox();
        float rL=rHit.position.x,rR=rL+rHit.size.x,rT=rHit.position.y,rB=rT+rHit.size.y;
        for (int v=0;v<m_enemyCount;++v) {
            if (v==r||!m_enemies[v]||!m_enemies[v]->isAlive()) continue;
            Enemy::State vs=m_enemies[v]->getState();
            if (vs==Enemy::State::Dead||vs==Enemy::State::Rolling) continue;
            sf::FloatRect vHit=m_enemies[v]->getHitBox();
            float vL=vHit.position.x,vR=vL+vHit.size.x,vT=vHit.position.y,vB=vT+vHit.size.y;
            if(!((rR>vL)&&(rL<vR)&&(rB>vT)&&(rT<vB))) continue;
            m_chainCount[r]++;
            int base=randomScore(100,500), bonus=static_cast<int>(base*0.1f*(m_chainCount[r]-1));
            m_score+=base+bonus;
            if (m_powerUpCount<MAX_POWERUPS) {
                int ti=std::rand()%static_cast<int>(PowerUp::Type::Count_);
                m_powerUps[m_powerUpCount++]=new PowerUp(m_enemies[v]->getPosition(),
                                                          static_cast<PowerUp::Type>(ti));
            }
            m_enemies[v]->setAlive(false);
        }
    }

    for (int i=0;i<m_powerUpCount;++i) {
        if (!m_powerUps[i]) continue;
        float px=m_powerUps[i]->getPosition().x, py=m_powerUps[i]->getPosition().y;
        m_powerUps[i]->update(dt);
        m_collider.resolve(*m_powerUps[i],m_platforms,m_platformCount,px,py);
    }

    if (m_player) {
        sf::FloatRect pHit=m_player->getHitBox();
        float pL=pHit.position.x,pR=pL+pHit.size.x,pT=pHit.position.y,pB=pT+pHit.size.y;
        for (int i=0;i<m_powerUpCount;++i) {
            if (!m_powerUps[i]||!m_powerUps[i]->isAlive()) continue;
            sf::FloatRect h=m_powerUps[i]->getHitBox();
            float hL=h.position.x,hR=hL+h.size.x,hT=h.position.y,hB=hT+h.size.y;
            if (!((pR>hL)&&(pL<hR)&&(pB>hT)&&(pT<hB))) continue;
            activatePowerUp(m_powerUps[i]->getType());
            m_powerUps[i]->setAlive(false);
        }
    }
    { int w=0;
      for (int r=0;r<m_powerUpCount;++r) {
        if(m_powerUps[r]&&m_powerUps[r]->isAlive()) m_powerUps[w++]=m_powerUps[r];
        else { delete m_powerUps[r]; m_powerUps[r]=nullptr; }
      } m_powerUpCount=w; }

    updatePowerUpTimers(dt);

    { int w=0;
      for (int r=0;r<m_enemyCount;++r) {
        Enemy* e=m_enemies[r];
        bool keep=e&&e->isAlive()&&e->getState()!=Enemy::State::Dead;
        if (keep) { m_enemies[w]=e; m_chainCount[w]=m_chainCount[r]; ++w; }
        else { delete e; m_enemies[r]=nullptr; }
      }
      for (int i=w;i<m_enemyCount;++i) { m_enemies[i]=nullptr; m_chainCount[i]=0; }
      m_enemyCount=w; }

    if (!m_gameOver&&m_player&&!m_player->isInvincible()
        &&m_collider.checkEnemyContact(*m_player,m_enemies,m_enemyCount)) {
        m_player->loseLife();
        std::cout<<"[PlayState] Player lost a life. Lives left: "<<m_player->getLives()<<"\n";
        if (m_player->getLives()<=0) {
            m_gameOver=true;
            saveScore(m_manager->getCurrentUserName(),m_score,m_currentLevel);
            m_manager->pushState(new GameOverState());
        } else {
            m_player->respawn(m_playerSpawn);
        }
    }
}

void PlayState::draw(sf::RenderWindow& window) {
    if (m_backgroundLoaded) window.draw(m_backgroundSprite);
    else {
        sf::RectangleShape fb({WINDOW_WIDTH,WINDOW_HEIGHT});
        fb.setFillColor(sf::Color(20,30,50)); window.draw(fb);
    }
    for (int i=0;i<m_platformCount;++i) m_platforms[i]->draw(window);
    for (int i=0;i<m_enemyCount;++i)    if(m_enemies[i])    m_enemies[i]->draw(window);
    for (int i=0;i<m_projectileCount;++i) if(m_projectiles[i]) m_projectiles[i]->draw(window);
    for (int i=0;i<m_powerUpCount;++i)  if(m_powerUps[i])   m_powerUps[i]->draw(window);
    if (m_player) m_player->draw(window);
    for (int i=0;i<m_hitFlashCount;++i) if(m_hitFlashes[i]) m_hitFlashes[i]->draw(window);

    if (m_showHitboxes) {
        if (m_player) m_player->drawHitBoxDebug(window,sf::Color::Green);
        for (int i=0;i<m_enemyCount;++i)      if(m_enemies[i])     m_enemies[i]->drawHitBoxDebug(window,sf::Color::Red);
        for (int i=0;i<m_projectileCount;++i)  if(m_projectiles[i]) m_projectiles[i]->drawHitBoxDebug(window,sf::Color::Yellow);
        for (int i=0;i<m_powerUpCount;++i)     if(m_powerUps[i])    m_powerUps[i]->drawHitBoxDebug(window,sf::Color::Magenta);
        for (int i=0;i<m_platformCount;++i) {
            if (!m_platforms[i]) continue;
            for (int b=0;b<m_platforms[i]->getHitboxCount();++b) {
                sf::FloatRect r=m_platforms[i]->getBounds(b);
                sf::RectangleShape box({r.size.x,r.size.y});
                box.setPosition({r.position.x,r.position.y});
                box.setFillColor(sf::Color::Transparent);
                box.setOutlineColor(sf::Color::Blue);
                box.setOutlineThickness(1.f);
                window.draw(box);
            }
        }
    }
    drawHUD(window);
}

void PlayState::drawHUD(sf::RenderWindow& window) {
    if (!m_hudFontLoaded) return;
    const float HUD_Y=14.f, LEFT_X=18.f, RIGHT_PAD=18.f, ICON_SIZE=16.f, ICON_TEXT_GAP=6.f;
    const unsigned int TEXT_SIZE=14;

    { sf::Text t(m_hudFont,"SCORE "+std::to_string(m_score),TEXT_SIZE);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black);
      t.setOutlineThickness(2.f); t.setPosition({LEFT_X,HUD_Y}); window.draw(t); }

    { float ly=HUD_Y+22.f; int lives=(m_player?m_player->getLives():0);
      if (m_heartLoaded) {
        sf::Sprite h(m_heartTexture); auto ts=m_heartTexture.getSize();
        if(ts.x>0&&ts.y>0) h.setScale({ICON_SIZE/static_cast<float>(ts.x),ICON_SIZE/static_cast<float>(ts.y)});
        h.setPosition({LEFT_X,ly}); window.draw(h); }
      sf::Text t(m_hudFont,std::to_string(lives),TEXT_SIZE);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black);
      t.setOutlineThickness(2.f); t.setPosition({LEFT_X+ICON_SIZE+ICON_TEXT_GAP,ly}); window.draw(t); }

    { std::string gs=std::to_string(m_gems);
      sf::Text t(m_hudFont,gs,TEXT_SIZE);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      auto tb=t.getLocalBounds();
      float tx=800.f-RIGHT_PAD-tb.size.x-tb.position.x; t.setPosition({tx,HUD_Y}); window.draw(t);
      if (m_diamondLoaded) {
        sf::Sprite d(m_diamondTexture); auto ts=m_diamondTexture.getSize();
        if(ts.x>0&&ts.y>0) d.setScale({ICON_SIZE/static_cast<float>(ts.x),ICON_SIZE/static_cast<float>(ts.y)});
        d.setPosition({tx-ICON_SIZE-ICON_TEXT_GAP,HUD_Y}); window.draw(d); } }

    { std::string ls="LEVEL "+std::to_string(m_currentLevel)+"/"+std::to_string(m_totalLevels);
      sf::Text t(m_hudFont,ls,TEXT_SIZE);
      t.setFillColor(sf::Color::White); t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2.f);
      auto tb=t.getLocalBounds();
      t.setPosition({(800.f-tb.size.x)/2.f-tb.position.x,HUD_Y}); window.draw(t); }

    drawPowerUpHUD(window);
}

int PlayState::randomScore(int lo, int hi) const {
    return lo + (std::rand() % (hi-lo+1));
}

void PlayState::loadPowerUpIcons() {
    auto tryLoad=[](sf::Texture& tex,const char* path,bool& flag){
        if(std::FILE* f=std::fopen(path,"rb")){std::fclose(f);flag=tex.loadFromFile(path);}else flag=false;};
    tryLoad(m_puIconSpeed,   "assets/sprites/powerup_speed.png",   m_puIconSpeedLoaded);
    tryLoad(m_puIconSnowball,"assets/sprites/powerup_snowball.png",m_puIconSnowballLoaded);
    tryLoad(m_puIconDistance,"assets/sprites/powerup_distance.png",m_puIconDistanceLoaded);
    tryLoad(m_puIconBalloon, "assets/sprites/powerup_balloon.png", m_puIconBalloonLoaded);
}

void PlayState::activatePowerUp(PowerUp::Type type) {
    switch (type) {
        case PowerUp::Type::SpeedBoost:
            m_speedActive=true; m_speedTimer=15.f;
            if(m_player) m_player->setSpeedMultiplier(1.5f); break;
        case PowerUp::Type::SnowballPower:
            m_snowballPowerActive=true;
            for(int i=0;i<m_enemyCount;++i) if(m_enemies[i]) m_enemies[i]->setOneHitEncase(true); break;
        case PowerUp::Type::DistanceIncrease:
            m_distanceActive=true; break;
        case PowerUp::Type::BalloonMode:
            m_balloonActive=true; m_balloonTimer=10.f;
            if(m_player) m_player->setBalloonMode(true); break;
        default: return;
    }
    m_displayedType=type; m_hasDisplayed=true;
    std::cout<<"[PlayState] Activated power-up: "<<PowerUp::typeName(type)<<"\n";
}

void PlayState::updatePowerUpTimers(float dt) {
    if (m_speedActive) {
        m_speedTimer-=dt;
        if (m_speedTimer<=0.f) {
            m_speedActive=false; m_speedTimer=0.f;
            if(m_player) m_player->setSpeedMultiplier(1.f);
            if(m_hasDisplayed&&m_displayedType==PowerUp::Type::SpeedBoost) {
                if      (m_balloonActive)       m_displayedType=PowerUp::Type::BalloonMode;
                else if (m_snowballPowerActive) m_displayedType=PowerUp::Type::SnowballPower;
                else if (m_distanceActive)      m_displayedType=PowerUp::Type::DistanceIncrease;
                else                            m_hasDisplayed=false;
            }
        }
    }
    if (m_balloonActive) {
        m_balloonTimer-=dt;
        if (m_balloonTimer<=0.f) {
            m_balloonActive=false; m_balloonTimer=0.f;
            if(m_player) m_player->setBalloonMode(false);
            if(m_hasDisplayed&&m_displayedType==PowerUp::Type::BalloonMode) {
                if      (m_speedActive)         m_displayedType=PowerUp::Type::SpeedBoost;
                else if (m_snowballPowerActive) m_displayedType=PowerUp::Type::SnowballPower;
                else if (m_distanceActive)      m_displayedType=PowerUp::Type::DistanceIncrease;
                else                            m_hasDisplayed=false;
            }
        }
    }
}

void PlayState::drawPowerUpHUD(sf::RenderWindow& window) {
    if (!m_hasDisplayed) return;
    const float Y=568.f, ICON=22.f, BAR_W=160.f, BAR_H=10.f, GAP=8.f;
    float totalW=ICON+GAP+BAR_W, startX=(800.f-totalW)/2.f;
    float barX=startX+ICON+GAP, barY=Y+(ICON-BAR_H)/2.f;

    sf::Texture* tex=nullptr; bool loaded=false;
    float fill=1.f; sf::Color col=sf::Color::White;
    switch(m_displayedType){
        case PowerUp::Type::SpeedBoost:      tex=&m_puIconSpeed;    loaded=m_puIconSpeedLoaded;
            fill=(m_speedTimer<=0.f)?0.f:(m_speedTimer/30.f); col=sf::Color(120,200,255); break;
        case PowerUp::Type::SnowballPower:   tex=&m_puIconSnowball; loaded=m_puIconSnowballLoaded;
            fill=1.f; col=sf::Color(255,120,200); break;
        case PowerUp::Type::DistanceIncrease:tex=&m_puIconDistance; loaded=m_puIconDistanceLoaded;
            fill=1.f; col=sf::Color(80,140,255); break;
        case PowerUp::Type::BalloonMode:     tex=&m_puIconBalloon;  loaded=m_puIconBalloonLoaded;
            fill=(m_balloonTimer<=0.f)?0.f:(m_balloonTimer/20.f); col=sf::Color(180,220,255); break;
        default: return;
    }
    if (loaded&&tex) {
        sf::Sprite icon(*tex); auto ts=tex->getSize();
        if(ts.x>0&&ts.y>0) icon.setScale({ICON/static_cast<float>(ts.x),ICON/static_cast<float>(ts.y)});
        icon.setPosition({startX,Y}); window.draw(icon); }
    sf::RectangleShape bg({BAR_W,BAR_H}); bg.setPosition({barX,barY});
    bg.setFillColor(sf::Color(20,20,30,200)); bg.setOutlineColor(sf::Color::Black);
    bg.setOutlineThickness(1.f); window.draw(bg);
    if (fill>0.f) {
        sf::RectangleShape f({BAR_W*fill,BAR_H}); f.setPosition({barX,barY});
        f.setFillColor(col); window.draw(f); }
}