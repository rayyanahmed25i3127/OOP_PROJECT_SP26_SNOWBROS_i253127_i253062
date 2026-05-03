#pragma once

#include "states/GameState.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

// Yeh pause menu jo ek naye design ke sath hai jis mein "pause_bg.png" background aur 
// iOS-style ke glassy buttons hain (ios 26 inspired). Is mein paanch rect ki shakal 
// ke buttons hain jin par mouse le jaane se woh "zoom" hote hain yani k hover effect hai, aur in ke zariye 
// aap game dobara shuru kar sakte hain, main menu par ja sakte hain, ya game band kar 
// sakte hain. Jab yeh menu khulta hai toh purani game wahi ruk jati hai jab tak aap "Resume" na press kardein ok bye.
class PauseState : public GameState {
public:
    static const int NUM_BUTTONS = 5;

private:
    enum class Action { Resume, MainMenu, Shop, Logout, Exit };

    struct Button {
        sf::Text text;
        sf::RectangleShape background;
        Action action;
        sf::Vector2f center;
        float baseWidth;
        float baseHeight;
        float currentScale;
        float targetScale;

        Button(const sf::Font& font);
        void configure(sf::Vector2f centerPos, float w, float h,
                       const std::string& label, Action act);
        void update(float dt);
        void draw(sf::RenderWindow& window) const;
    };

    sf::Font m_font;
    sf::Text m_title;

   //bg
    sf::Texture m_bgTexture;
    sf::Sprite m_bgSprite;
    bool m_bgLoaded;

    //clicksound
    sf::SoundBuffer m_clickSoundBuffer;
    sf::Sound m_clickSound;
    bool m_clickSoundLoaded;

    Button* m_buttons[NUM_BUTTONS];
    int m_selectedIndex;

    //helpers
    void setHovered(int index);
    int buttonAtPoint(sf::Vector2f point) const;
    void activateButton(int index);

public:
    PauseState();
    ~PauseState();
    bool isTransparent() const override { return true; }
    PauseState(const PauseState&) = delete;
    PauseState& operator=(const PauseState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onEnter() override;
};