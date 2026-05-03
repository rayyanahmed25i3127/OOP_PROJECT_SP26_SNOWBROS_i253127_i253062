#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>

class ShopState : public GameState {
private:
    static const int MAX_ITEMS = 5;

    
    sf::Texture  m_bgTexture;
    sf::Sprite*  m_bgSprite;
    bool         m_bgLoaded;

    
    sf::Font     m_font;

    //gem display topright (diamond ki PNG + count below it, like howmany diamonds)  
    sf::Texture  m_diamondTexture;
    sf::Sprite*  m_diamondSprite;
    bool         m_diamondLoaded;
    sf::Text*    m_gemCountText;
    int          m_gemCount;        // random value for now

    //back arrow button
    sf::Text*    m_backText;
    bool         m_backHovered;
    sf::FloatRect m_backBounds;

    //info / feedback text (shown after buy attempt)
    sf::Text*    m_infoText;

    // Shop item data
    sf::Texture   m_itemTextures[MAX_ITEMS];
    sf::Sprite*   m_itemSprites[MAX_ITEMS];
    sf::Text*     m_itemNameTexts[MAX_ITEMS];
    sf::Text*     m_itemPriceTexts[MAX_ITEMS];
    sf::FloatRect m_cardBounds[MAX_ITEMS];
    bool          m_itemHovered[MAX_ITEMS];
    sf::Vector2f  m_cardPositions[MAX_ITEMS];   // top-left corner of each card

   //Card dimensions
    static const float CARD_H;     // 178
    static const float IMG_SIZE;   // 88 all sprites uniformly scaled to this

    //helpers 
    void drawCapsule(sf::RenderWindow& window,
                     float x, float y,
                     float width, float height,
                     sf::Color fillColor,
                     sf::Color outlineColor    = sf::Color::Transparent,
                     float     outlineThickness = 0.f);

    void drawCard(sf::RenderWindow& window, int index);
    void tryBuy(int index);

//multiplayer popup
    bool  m_showPopup;
    int   m_popupItemIndex;
    sf::FloatRect m_popupBtnP1;
    sf::FloatRect m_popupBtnP2;
    sf::FloatRect m_popupBtnCancel;
    void drawPopup(sf::RenderWindow& window);
    void doBuyForPlayer(int itemIndex, int playerNum);

public:
    ShopState();
    ~ShopState();

    // shopstate draws its own full background jo k transparent nai hai
    bool isTransparent() const override { return false; }

    void onEnter()                           override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
};