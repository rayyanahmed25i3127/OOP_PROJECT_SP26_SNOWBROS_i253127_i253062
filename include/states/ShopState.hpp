#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>

class ShopState : public GameState {
private:
    static const int MAX_ITEMS = 5;

    // ── Background ────────────────────────────────────────────────────────────
    sf::Texture  m_bgTexture;
    sf::Sprite*  m_bgSprite;
    bool         m_bgLoaded;

    // ── Font ──────────────────────────────────────────────────────────────────
    sf::Font     m_font;

    // ── Gem display — top-right (diamond PNG + count below it) ────────────────
    sf::Texture  m_diamondTexture;
    sf::Sprite*  m_diamondSprite;
    bool         m_diamondLoaded;
    sf::Text*    m_gemCountText;
    int          m_gemCount;        // random value for now

    // ── Back arrow button — top-left ──────────────────────────────────────────
    sf::Text*    m_backText;
    bool         m_backHovered;
    sf::FloatRect m_backBounds;

    // ── Info / feedback text (shown after buy attempt) ─────────────────────────
    sf::Text*    m_infoText;

    // ── Shop item data (fixed arrays — no vector) ──────────────────────────────
    sf::Texture   m_itemTextures[MAX_ITEMS];
    sf::Sprite*   m_itemSprites[MAX_ITEMS];
    sf::Text*     m_itemNameTexts[MAX_ITEMS];
    sf::Text*     m_itemPriceTexts[MAX_ITEMS];
    sf::FloatRect m_cardBounds[MAX_ITEMS];
    bool          m_itemHovered[MAX_ITEMS];
    sf::Vector2f  m_cardPositions[MAX_ITEMS];   // top-left corner of each card

    // ── Card dimensions ────────────────────────────────────────────────────────
    static const float CARD_W;     // 140
    static const float CARD_H;     // 178
    static const float IMG_SIZE;   // 88 — all sprites uniformly scaled to this

    // ── Helpers ───────────────────────────────────────────────────────────────
    // TriangleFan capsule — no seam, no vector, no optional
    void drawCapsule(sf::RenderWindow& window,
                     float x, float y,
                     float width, float height,
                     sf::Color fillColor,
                     sf::Color outlineColor    = sf::Color::Transparent,
                     float     outlineThickness = 0.f);

    void drawCard(sf::RenderWindow& window, int index);
    void tryBuy(int index);

public:
    ShopState();
    ~ShopState();

    // ShopState draws its own full background → not transparent
    bool isTransparent() const override { return false; }

    void onEnter()                           override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
};