#pragma once
#include "enemies/Enemy.hpp"

// Identical to FlyngFoogaFoog (blue) — only asset filenames differ (red).
class FlyngFoogaFoogRed : public Enemy {
private:
    bool  m_isFlying;
    float m_flyTimer;
    float m_walkTimer;

    sf::Texture m_flyTextures[3];
    bool  m_flyLoaded;
    int   m_flyFrame;
    float m_flyFrameTimer;

    sf::Texture m_fooga25Texture;
    sf::Texture m_fooga50Texture;
    sf::Texture m_fooga75Texture;
    bool m_fooga25Loaded;
    bool m_fooga50Loaded;
    bool m_fooga75Loaded;

    static constexpr float FLY_DURATION   = 10.0f;
    static constexpr float WALK_DURATION  = 5.0f;
    static constexpr float FLY_FRAME_TIME = 0.33f;

    void updateAI(float dt) override;
    void updateAnimation(float dt) override;
    void applyStateSprite() override;
    float getPartialEncaseDuration() const override { return 1.5f; }

public:
    FlyngFoogaFoogRed(sf::Vector2f pos);
    void update(float dt) override;
    bool isFlying() const { return m_isFlying; }
};