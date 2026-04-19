#include <SFML/Graphics.hpp>
#include <vector>

int main()
{
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Snow Bros");

    // Player
    sf::CircleShape player(25.f);
    player.setFillColor(sf::Color::Green);
    player.setPosition(sf::Vector2f(400.f, 500.f));

    float speed = 0.5f;

    std::vector<sf::CircleShape> enemies;

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Movement (SFML 3)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            player.move(sf::Vector2f(-speed, 0));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            player.move(sf::Vector2f(speed, 0));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            player.move(sf::Vector2f(0, -speed));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            player.move(sf::Vector2f(0, speed));

        // Render
        window.clear();
        window.draw(player);
        window.display();
    }

    return 0;
}