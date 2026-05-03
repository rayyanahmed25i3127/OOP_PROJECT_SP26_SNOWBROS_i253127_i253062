#include "states/ShopState.hpp"
#include "states/StateManager.hpp"
#include "audio/AudioManager.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

const float ShopState::CARD_W   = 140.f;
const float ShopState::CARD_H   = 178.f;
const float ShopState::IMG_SIZE =  88.f;

static const int   NUM_ITEMS   = 5;
static const char* ITEM_FILES[NUM_ITEMS] = {
    "assets/sprites/powerup_balloon.png",
    "assets/sprites/powerup_distance.png",
    "assets/sprites/powerup_snowball.png",
    "assets/sprites/powerup_speed.png",
    "assets/sprites/heart.png"
};
static const char* ITEM_NAMES[NUM_ITEMS] = {
    "Balloon Mode",
    "Distance Increase",
    "Snowball Power",
    "Speed Boost",
    "Extra Life"
};
static const int ITEM_PRICES[NUM_ITEMS] = { 35, 25, 30, 20, 50 };

static bool isAlreadyBought(int index, StateManager* mgr) {
    if (!mgr) return false;
    PlayerProgress& prog = mgr->getProgress();
    switch (index) {
        case 0: return prog.pendingBalloon;
        case 1: return prog.pendingDistance;
        case 2: return prog.pendingSnowball;
        case 3: return prog.pendingSpeed;
        case 4: return false;  // Extra Life: always buyable
        default: return false;
    }
}

ShopState::ShopState()
    : m_bgSprite(nullptr),     m_bgLoaded(false)
    , m_diamondSprite(nullptr), m_diamondLoaded(false)
    , m_gemCountText(nullptr),  m_gemCount(0)
    , m_backText(nullptr),      m_backHovered(false)
    , m_infoText(nullptr)
{
    for (int i = 0; i < MAX_ITEMS; ++i) {
        m_itemSprites[i]    = nullptr;
        m_itemNameTexts[i]  = nullptr;
        m_itemPriceTexts[i] = nullptr;
        m_itemHovered[i]    = false;
        m_cardPositions[i]  = {0.f, 0.f};
    }
}

ShopState::~ShopState() {
    delete m_bgSprite; delete m_diamondSprite;
    delete m_gemCountText; delete m_backText; delete m_infoText;
    for (int i = 0; i < MAX_ITEMS; ++i) {
        delete m_itemSprites[i];
        delete m_itemNameTexts[i];
        delete m_itemPriceTexts[i];
    }
}

void ShopState::drawCapsule(sf::RenderWindow& window,
                             float x, float y, float width, float height,
                             sf::Color fillColor, sf::Color outlineColor,
                             float outlineThickness)
{
    static const int SEGS      = 28;
    static const int MAX_VERTS = 2 * SEGS + 4;

    auto build = [&](float bx, float by, float bw, float bh,
                     sf::Color col, sf::Vertex out[]) -> int {
        float r=bh/2.f, lcx=bx+r, rcx=bx+bw-r, midY=by+r; int idx=0;
        out[idx++]={{bx+bw/2.f,midY},col};
        for(int i=0;i<=SEGS;++i){float a=-3.14159265f/2.f+3.14159265f*i/SEGS;
            out[idx++]={{rcx+r*std::cos(a),midY+r*std::sin(a)},col};}
        for(int i=0;i<=SEGS;++i){float a=3.14159265f/2.f+3.14159265f*i/SEGS;
            out[idx++]={{lcx+r*std::cos(a),midY+r*std::sin(a)},col};}
        out[idx++]=out[1]; return idx;
    };
    sf::Vertex verts[MAX_VERTS]; int cnt=0;
    if(outlineThickness>0.f&&outlineColor.a>0){
        cnt=build(x-outlineThickness,y-outlineThickness,
                  width+2.f*outlineThickness,height+2.f*outlineThickness,outlineColor,verts);
        window.draw(verts,static_cast<std::size_t>(cnt),sf::PrimitiveType::TriangleFan);}
    cnt=build(x,y,width,height,fillColor,verts);
    window.draw(verts,static_cast<std::size_t>(cnt),sf::PrimitiveType::TriangleFan);
}

void ShopState::tryBuy(int index) {
    if (index < 0 || index >= MAX_ITEMS || !m_infoText) return;

    PlayerProgress& prog = m_manager->getProgress();
    // as many times as the player has gems.
    bool alreadyBought = false;
    switch (index) {
        case 0: alreadyBought = prog.pendingBalloon; break;
        case 1: alreadyBought = prog.pendingDistance; break;
        case 2: alreadyBought = prog.pendingSnowball; break;
        case 3: alreadyBought = prog.pendingSpeed; break;
        case 4: alreadyBought = false; break; // always purchasable
    }
    
    if (alreadyBought) {
        m_infoText->setString("Already purchased!");
        m_infoText->setFillColor(sf::Color(255, 200, 80));
        sf::FloatRect ib = m_infoText->getLocalBounds();
        m_infoText->setOrigin({ib.position.x + ib.size.x/2.f, 0.f});
        m_infoText->setPosition({400.f, 560.f});
        return;
    }

    int price = ITEM_PRICES[index];
    if (m_gemCount < price) {
        m_infoText->setString("Not enough gems!");
        m_infoText->setFillColor(sf::Color(255, 120, 100));
    } else {
        m_gemCount -= price;
        prog.gems = m_gemCount;

        switch (index) {
            case 0: prog.pendingBalloon = true; break;
            case 1: prog.pendingDistance = true; break;
            case 2: prog.pendingSnowball = true; break;
            case 3: prog.pendingSpeed = true; break;
            case 4: prog.pendingExtraLifeCount++; break;  // stackable
        }

        if (m_gemCountText) {
            m_gemCountText->setString(std::to_string(m_gemCount));
            const float panelX=638.f, panelY=8.f, panelW=152.f, panelH=80.f;
            sf::FloatRect tb=m_gemCountText->getLocalBounds();
            m_gemCountText->setOrigin({tb.position.x+tb.size.x/2.f, tb.position.y+tb.size.y/2.f});
            m_gemCountText->setPosition({panelX+panelW/2.f, panelY+panelH-14.f});
        }

        m_infoText->setString(std::string(ITEM_NAMES[index]) + " purchased!");
        m_infoText->setFillColor(sf::Color(100, 255, 160));
    }

    sf::FloatRect ib=m_infoText->getLocalBounds();
    m_infoText->setOrigin({ib.position.x+ib.size.x/2.f, 0.f});
    m_infoText->setPosition({400.f, 560.f});
}

void ShopState::onEnter() {
    AudioManager::get().playMenuMusic();
    static bool seeded=false;
    if(!seeded){std::srand(static_cast<unsigned>(std::time(nullptr)));seeded=true;}
    m_gemCount = m_manager->getProgress().gems;

    if (!m_font.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf"))
        std::cerr<<"[ShopState] Font failed\n";

    if (!m_bgTexture.loadFromFile("assets/sprites/shop_bg.jpeg")) {
        std::cerr<<"[ShopState] shop_bg.jpeg failed\n"; m_bgLoaded=false;
    } else {
        m_bgLoaded=true; delete m_bgSprite; m_bgSprite=new sf::Sprite(m_bgTexture);
        sf::Vector2u tex=m_bgTexture.getSize();
        m_bgSprite->setScale({800.f/static_cast<float>(tex.x),600.f/static_cast<float>(tex.y)});
    }

    const float panelX=638.f, panelY=8.f, panelW=152.f, panelH=80.f;

    if (!m_diamondTexture.loadFromFile("assets/sprites/diamond.png")) {
        std::cerr<<"[ShopState] diamond.png failed\n"; m_diamondLoaded=false;
    } else {
        m_diamondLoaded=true; delete m_diamondSprite; m_diamondSprite=new sf::Sprite(m_diamondTexture);
        const float maxDim=36.f; sf::Vector2u ts=m_diamondTexture.getSize();
        if(ts.x>0&&ts.y>0){ float s=std::min(maxDim/ts.x,maxDim/ts.y); m_diamondSprite->setScale({s,s}); }
        sf::FloatRect db=m_diamondSprite->getGlobalBounds();
        float imageAreaH=panelH-26.f;
        m_diamondSprite->setPosition({panelX+(panelW-db.size.x)/2.f, panelY+(imageAreaH-db.size.y)/2.f});
    }

    m_gemCountText=new sf::Text(m_font); m_gemCountText->setString(std::to_string(m_gemCount));
    m_gemCountText->setCharacterSize(22); m_gemCountText->setFillColor(sf::Color(180,230,255));
    { sf::FloatRect tb=m_gemCountText->getLocalBounds();
      m_gemCountText->setOrigin({tb.position.x+tb.size.x/2.f,tb.position.y+tb.size.y/2.f});
      m_gemCountText->setPosition({panelX+panelW/2.f,
          m_diamondLoaded ? panelY+panelH-14.f : panelY+panelH/2.f}); }

    m_backBounds=sf::FloatRect({12.f,10.f},{110.f,42.f});
    m_backText=new sf::Text(m_font); m_backText->setString("< BACK");
    m_backText->setCharacterSize(22); m_backText->setFillColor(sf::Color::White);
    { sf::FloatRect bb=m_backText->getLocalBounds();
      m_backText->setOrigin({bb.position.x+bb.size.x/2.f,bb.position.y+bb.size.y/2.f});
      m_backText->setPosition({12.f+110.f/2.f,10.f+42.f/2.f}); }

    m_infoText=new sf::Text(m_font); m_infoText->setString("");
    m_infoText->setCharacterSize(22); m_infoText->setFillColor(sf::Color::White);
    m_infoText->setPosition({400.f,560.f});

    const float ROW1_Y=185.f, ROW2_Y=ROW1_Y+CARD_H+18.f, CARD_GAP=22.f;
    float r1X=(800.f-(3.f*CARD_W+2.f*CARD_GAP))/2.f;
    m_cardPositions[0]={r1X,ROW1_Y};
    m_cardPositions[1]={r1X+CARD_W+CARD_GAP,ROW1_Y};
    m_cardPositions[2]={r1X+2.f*(CARD_W+CARD_GAP),ROW1_Y};
    float r2X=(800.f-(2.f*CARD_W+CARD_GAP))/2.f;
    m_cardPositions[3]={r2X,ROW2_Y};
    m_cardPositions[4]={r2X+CARD_W+CARD_GAP,ROW2_Y};

    for (int i=0;i<MAX_ITEMS;++i) {
        float cx=m_cardPositions[i].x, cy=m_cardPositions[i].y;
        m_cardBounds[i]=sf::FloatRect({cx,cy},{CARD_W,CARD_H});
        if(!m_itemTextures[i].loadFromFile(ITEM_FILES[i]))
            std::cerr<<"[ShopState] Failed: "<<ITEM_FILES[i]<<"\n";
        delete m_itemSprites[i]; m_itemSprites[i]=new sf::Sprite(m_itemTextures[i]);
        sf::Vector2u ts=m_itemTextures[i].getSize();
        if(ts.x>0&&ts.y>0){
            float scl=std::min(IMG_SIZE/static_cast<float>(ts.x),
                               IMG_SIZE/static_cast<float>(ts.y))*0.9f;
            m_itemSprites[i]->setScale({scl,scl});}
        { sf::FloatRect sb=m_itemSprites[i]->getLocalBounds();
          float sw=sb.size.x*m_itemSprites[i]->getScale().x;
          float sh=sb.size.y*m_itemSprites[i]->getScale().y;
          float izH=CARD_H*0.55f;
          m_itemSprites[i]->setPosition({cx+(CARD_W-sw)/2.f, cy+(izH-sh)/2.f}); }

        delete m_itemNameTexts[i]; m_itemNameTexts[i]=new sf::Text(m_font);
        m_itemNameTexts[i]->setString(ITEM_NAMES[i]);
        m_itemNameTexts[i]->setCharacterSize(18);
        m_itemNameTexts[i]->setFillColor(sf::Color::White);
        { sf::FloatRect nb=m_itemNameTexts[i]->getLocalBounds();
          m_itemNameTexts[i]->setOrigin({nb.position.x+nb.size.x/2.f,0.f});
          m_itemNameTexts[i]->setPosition({cx+CARD_W/2.f,cy+CARD_H*0.55f+6.f}); }

        delete m_itemPriceTexts[i]; m_itemPriceTexts[i]=new sf::Text(m_font);
        m_itemPriceTexts[i]->setString(std::to_string(ITEM_PRICES[i])+" gems");
        m_itemPriceTexts[i]->setCharacterSize(16);
        m_itemPriceTexts[i]->setFillColor(sf::Color(180,230,255));
        { sf::FloatRect pb=m_itemPriceTexts[i]->getLocalBounds();
          m_itemPriceTexts[i]->setOrigin({pb.position.x+pb.size.x/2.f,0.f});
          m_itemPriceTexts[i]->setPosition({cx+CARD_W/2.f,cy+CARD_H-28.f}); }
    }
}

void ShopState::drawCard(sf::RenderWindow& window, int index) {
    float x=m_cardPositions[index].x, y=m_cardPositions[index].y;
    bool  sold=isAlreadyBought(index, m_manager);

    // Glow
    sf::RectangleShape glow({CARD_W,CARD_H}); glow.setPosition({x,y});
    glow.setFillColor(sold
        ? sf::Color(40,40,40,15)
        : sf::Color(120,200,255,m_itemHovered[index]?40:20));
    window.draw(glow);

    // Card body â€” dimmed when sold
    sf::RectangleShape card({CARD_W,CARD_H}); card.setPosition({x,y});
    card.setFillColor(sold
        ? sf::Color(60,60,70,120)
        : sf::Color(180,220,255,m_itemHovered[index]?120:90));
    card.setOutlineThickness(2.f);
    card.setOutlineColor(sold
        ? sf::Color(100,100,110,150)
        : sf::Color(220,240,255,200));
    window.draw(card);

    if (m_itemSprites[index]) {
        m_itemSprites[index]->setColor(sold ? sf::Color(80,80,80,180) : sf::Color::White);
        window.draw(*m_itemSprites[index]);
        m_itemSprites[index]->setColor(sf::Color::White); // reset for next frame
    }

    // Divider
    sf::RectangleShape div({CARD_W-20.f,1.f});
    div.setFillColor(sf::Color(120,180,230,100));
    div.setPosition({x+10.f,y+CARD_H*0.55f+2.f}); window.draw(div);

    if (m_itemNameTexts[index]) window.draw(*m_itemNameTexts[index]);
    if (m_itemPriceTexts[index]) window.draw(*m_itemPriceTexts[index]);

    if (sold) {
        // Dark overlay panel
        sf::RectangleShape overlay({CARD_W,30.f});
        overlay.setPosition({x, y+CARD_H/2.f-15.f});
        overlay.setFillColor(sf::Color(0,0,0,160));
        window.draw(overlay);

        sf::Text soldText(m_font, "Alrd Bought", 13);
        soldText.setFillColor(sf::Color(255,200,80));
        soldText.setOutlineColor(sf::Color::Black);
        soldText.setOutlineThickness(2.f);
        auto tb=soldText.getLocalBounds();
        soldText.setPosition({x+(CARD_W-tb.size.x)/2.f-tb.position.x,
                              y+CARD_H/2.f-tb.size.y/2.f-tb.position.y});
        window.draw(soldText);
    }
}

void ShopState::handleEvent(const sf::Event& event) {
    if (const auto* key=event.getIf<sf::Event::KeyPressed>())
        if(key->code==sf::Keyboard::Key::Escape) m_manager->popState();

    if (const auto* mouse=event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button==sf::Mouse::Button::Left) {
            sf::Vector2f mp(static_cast<float>(mouse->position.x),
                            static_cast<float>(mouse->position.y));
            if (m_backBounds.contains(mp)) { m_manager->popState(); return; }
            for (int i=0;i<MAX_ITEMS;++i)
                if (m_cardBounds[i].contains(mp)) { tryBuy(i); break; }
        }
    }
}

void ShopState::update(float /*dt*/) {
    if (!m_window) return;
    sf::Vector2f mp=m_window->mapPixelToCoords(sf::Mouse::getPosition(*m_window));
    m_backHovered=m_backBounds.contains(mp);
    for (int i=0;i<MAX_ITEMS;++i) {
        // Don't show hover on sold items â€” clearer UX
        bool sold=isAlreadyBought(i,m_manager);
        m_itemHovered[i]=!sold && m_cardBounds[i].contains(mp);
    }
}

void ShopState::draw(sf::RenderWindow& window) {
    window.clear(sf::Color(5,10,30));
    if(m_bgLoaded&&m_bgSprite) window.draw(*m_bgSprite);

    // Back button
    sf::RectangleShape backBtn(sf::Vector2f(m_backBounds.size.x,m_backBounds.size.y));
    backBtn.setPosition(m_backBounds.position);
    backBtn.setFillColor(m_backHovered?sf::Color(80,160,255,230):sf::Color(20,60,130,210));
    backBtn.setOutlineThickness(2.f); backBtn.setOutlineColor(sf::Color::White);
    window.draw(backBtn);
    if(m_backText){m_backText->setFillColor(m_backHovered?sf::Color(255,255,180):sf::Color::White);
        window.draw(*m_backText);}

    // Gem panel
    sf::RectangleShape gemPanel({152.f,80.f}); gemPanel.setPosition({638.f,8.f});
    gemPanel.setFillColor(sf::Color(20,40,80,180)); gemPanel.setOutlineThickness(0.f);
    window.draw(gemPanel);
    if(m_diamondLoaded&&m_diamondSprite) window.draw(*m_diamondSprite);
    if(m_gemCountText) window.draw(*m_gemCountText);

    for(int i=0;i<MAX_ITEMS;++i) drawCard(window,i);

    if(m_infoText&&!m_infoText->getString().isEmpty()) window.draw(*m_infoText);
}