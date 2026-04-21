#include "Player.hpp"
#include <SFML/Window.hpp>//handles window
#include <SFML/Window/Keyboard.hpp>//handles inputs from keyboard

Player::Player(sf::Vector2f pos) : Entity(pos) {//base class is called
    // Shape (temporary instead of sprite)
    body.setSize(sf::Vector2f(40.f, 40.f));
    body.setFillColor(sf::Color::Blue);
    body.setPosition(pos);

    // Physics
    speed = 200.f;
    jumpForce = -400.f;
    gravity = 800.f;

    onGround = false;

    // Hitbox setup (initial size)
    hitBox.size = {40.f, 40.f};
    hitBox.position = pos;//hitbox is set on the position
}

void Player::handleInput() {
    velocity.x = 0.f;

    // move in left direction
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
    velocity.x = -speed;
}
//move in right direction
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
    velocity.x = speed;
}
//for jumping, && ground for player must on ground before jumping
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && onGround) {
    velocity.y = jumpForce;
    onGround = false;
}
}
//this is downward force (dt is time to ensure sma espeed in all pcs)
void Player::applyGravity(float dt) {
    velocity.y += gravity * dt;
}

void Player::update(float dt) {
    handleInput();
    applyGravity(dt);

    // Move
    position += velocity * dt;

    // SIMPLE GROUND COLLISION (for now)
    if (position.y >= 500.f) {   // ground level
        position.y = 500.f;//reset position
        //reset velocity
        velocity.y = 0.f;
        onGround = true;//set that it is on ground and ready to jump again
    }

    // Update visuals
    body.setPosition(position);

    // Update hitbox
    hitBox.position = position;
}
//drawing on screen
void Player::draw(sf::RenderWindow& window) {
    window.draw(body);

    // Debug hitbox (optional)
    // drawHitBoxDebug(window, sf::Color::Red);
}