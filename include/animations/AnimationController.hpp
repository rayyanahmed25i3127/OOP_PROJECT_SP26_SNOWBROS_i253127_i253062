#pragma once
#include "Animation.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief Manages multiple named animations for a single entity.
 *
 * Allows switching between animations (idle, walk, jump, etc.) and
 * automatically applies the current frame to a sprite.
 *
 * Usage:
 *   AnimationController controller;
 *   controller.addAnimation("idle", idleAnim);
 *   controller.addAnimation("walk", walkAnim);
 *   controller.setAnimation("idle");
 *   
 *   // In update loop:
 *   controller.update(dt);
 *   controller.applyToSprite(mySprite);
 */
class AnimationController {
private:
    static const int MAX_ANIMATIONS = 16;
    
    struct AnimationEntry {
        std::string name;
        Animation animation;
        bool isValid;
        
        AnimationEntry() : isValid(false) {}
    };
    
    AnimationEntry m_animations[MAX_ANIMATIONS];
    int m_animationCount;
    int m_currentIndex;  // Index of currently playing animation (-1 if none)

public:
    AnimationController();

    /**
     * @brief Add a named animation to the controller.
     * @param name Animation name (e.g., "idle", "walk", "jump")
     * @param anim The animation object
     */
    void addAnimation(const std::string& name, const Animation& anim);

    /**
     * @brief Switch to a different animation by name.
     * @param name Animation name to switch to
     * @param restart If true, restarts animation even if already playing
     * @return True if animation was found and switched
     */
    bool setAnimation(const std::string& name, bool restart = false);

    /**
     * @brief Update the current animation.
     * @param dt Delta time in seconds
     */
    void update(float dt);

    /**
     * @brief Apply current animation frame to a sprite.
     * @param sprite The sprite to update
     */
    void applyToSprite(sf::Sprite& sprite);

    /**
     * @brief Get name of currently playing animation.
     */
    std::string getCurrentAnimationName() const;

    /**
     * @brief Check if current animation has finished (for non-looping anims).
     */
    bool hasCurrentAnimationFinished() const;

    /**
     * @brief Get pointer to animation by name (for advanced control).
     */
    Animation* getAnimation(const std::string& name);
};