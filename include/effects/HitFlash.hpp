#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// Yeh code ek visual effect banata hai jo attack ball ke enemy se takrane 
// par nazar aata hai. Jab ball hit karti hai, toh woh foran khatam ho jati 
// hai aur wahan ek chota sa chamakdar "flash" peda hota hai jo sirf 100ms ke 
// liye rehta hai. Is effect ke liye wahi purana sprite chote size mein use kiya 
// gaya hai jo halka sa rotate hota hai taake impact ka asar zyada behtar mehsoos 
// ho. Yeh sirf dikhawe ke liye hai aur iska physics se koi talluq nahi hai.
class HitFlash {
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_textureLoaded;

    float m_lifetime;     // in mili secnds
    float m_maxLifetime;  // reset value
    bool  m_alive;

    sf::Vector2f m_position;

public:
    explicit HitFlash(sf::Vector2f pos,
                      const std::string& texturePath = "assets/sprites/hit_flash_blue.png");

    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool isAlive() const { return m_alive; }
};