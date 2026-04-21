#include <SFML/Graphics.hpp>
#include "../include/Player.h"
#include "../include/Platform.h"

int main() {
    // Create window (SFML 3 style)
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Snow Bros");
    window.setFramerateLimit(60);

    Player player({100.f, 100.f});
    Platform ground({800.f, 50.f}, {0.f, 550.f});
    Platform p1({200.f, 20.f}, {100.f, 400.f});
    Platform p2({150.f, 20.f}, {400.f, 300.f});

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Update
        player.update(dt);

        // Draw
        window.clear(sf::Color::Black);
        ground.draw(window);
        p1.draw(window);
        p2.draw(window);

        player.draw(window);
        window.display();
    }

    return 0;
}