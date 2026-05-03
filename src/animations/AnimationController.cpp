#include "animations/AnimationController.hpp"
#include <iostream>

AnimationController::AnimationController()
    : m_animationCount(0)
    , m_currentIndex(-1)
{}

void AnimationController::addAnimation(const std::string& name, const Animation& anim) {
    if (m_animationCount >= MAX_ANIMATIONS) {
        std::cerr << "[AnimationController] Max animations reached\n";
        return;
    }

    m_animations[m_animationCount].name = name;
    m_animations[m_animationCount].animation = anim;
    m_animations[m_animationCount].isValid = true;
    m_animationCount++;
}

bool AnimationController::setAnimation(const std::string& name, bool restart) {
    for (int i = 0; i < m_animationCount; ++i) {
        if (m_animations[i].isValid && m_animations[i].name == name) {
            // Already playing this animation?
            if (m_currentIndex == i && !restart) {
                return true; 
            }

            // Switch to new animation
            m_currentIndex = i;
            m_animations[i].animation.reset();
            m_animations[i].animation.play();
            return true;
        }
    }

    std::cerr << "[AnimationController] Animation '" << name << "' not found\n";
    return false;
}

void AnimationController::update(float dt) {
    if (m_currentIndex >= 0 && m_currentIndex < m_animationCount) {
        m_animations[m_currentIndex].animation.update(dt);
    }
}

void AnimationController::applyToSprite(sf::Sprite& sprite) {
    if (m_currentIndex >= 0 && m_currentIndex < m_animationCount) {
        sf::IntRect frame = m_animations[m_currentIndex].animation.getCurrentFrame();
        sprite.setTextureRect(frame);
    }
}

std::string AnimationController::getCurrentAnimationName() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_animationCount) {
        return m_animations[m_currentIndex].name;
    }
    return "";
}

bool AnimationController::hasCurrentAnimationFinished() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_animationCount) {
        return m_animations[m_currentIndex].animation.hasFinished();
    }
    return false;
}

Animation* AnimationController::getAnimation(const std::string& name) {
    for (int i = 0; i < m_animationCount; ++i) {
        if (m_animations[i].isValid && m_animations[i].name == name) {
            return &m_animations[i].animation;
        }
    }
    return nullptr;
}