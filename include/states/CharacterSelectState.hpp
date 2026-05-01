#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>

class CharacterSelectState : public GameState {
public:
    // PUBLIC so the anonymous namespace in the .cpp can reference
    // CharacterSelectState::NUM_CHARS without a C++(265) access error.
    static const int NUM_CHARS = 3;

private:
    sf::Texture  m_bgTexture;
    sf::Sprite*  m_bg;

    sf::Texture  m_textures[NUM_CHARS];
    sf::Sprite*  m_sprites[NUM_CHARS];

    sf::RectangleShape m_cards[NUM_CHARS];
    sf::FloatRect      m_bounds[NUM_CHARS];

    sf::Font    m_font;
    bool        m_fontLoaded;
    sf::Text*   m_title;
    sf::Text*   m_labels[NUM_CHARS];

    int  m_hoveredIndex;

    static constexpr float WINDOW_W    = 800.f;
    static constexpr float WINDOW_H    = 600.f;
    static constexpr float CARD_W      = 200.f;
    static constexpr float CARD_H      = 270.f;
    static constexpr float CARD_GAP    = 28.f;
    static constexpr float CARD_TOP_Y  = 175.f;
    static constexpr float IMG_SIZE    = 180.f;
    static constexpr float IMG_PAD_TOP = 10.f;

    void buildLayout();
    void updateHover(sf::Vector2f mousePos);

public:
    CharacterSelectState();
    ~CharacterSelectState();

    void onEnter()  override;
    void onExit()   override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};