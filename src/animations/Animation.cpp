#include "animations/Animation.hpp"
#include <iostream>

Animation::Animation(float frameDuration, bool looping)
    : m_frameCount(0)
    , m_frameDuration(frameDuration)
    , m_currentTime(0.f)
    , m_currentFrameIndex(0)
    , m_isLooping(looping)
    , m_isPlaying(false)
    , m_hasFinished(false)
{}

void Animation::addFrame(const sf::IntRect& rect) {
    if (m_frameCount >= MAX_FRAMES) {
        std::cerr << "[Animation] Max frames reached (" << MAX_FRAMES << ")\n";
        return;
    }
    m_frames[m_frameCount] = rect;
    m_frameCount++;
}

void Animation::update(float dt) {
    if (!m_isPlaying || m_frameCount == 0) return;
    if (m_hasFinished && !m_isLooping) return;

    m_currentTime += dt;

    // Advance to next frame(s) if enough time has passed
    while (m_currentTime >= m_frameDuration) {
        m_currentTime -= m_frameDuration;
        m_currentFrameIndex++;

        // Handle end of animation
        if (m_currentFrameIndex >= m_frameCount) {
            if (m_isLooping) {
                m_currentFrameIndex = 0;  // Loop back to start
            } else {
                m_currentFrameIndex = m_frameCount - 1;  // Stay on last frame
                m_hasFinished = true;
                m_isPlaying = false;
                break;
            }
        }
    }
}

sf::IntRect Animation::getCurrentFrame() const {
    if (m_frameCount == 0) {
        return sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(1, 1));  // it will fall back if not on the platform 
    }
    return m_frames[m_currentFrameIndex];
}

void Animation::reset() {
    m_currentFrameIndex = 0;
    m_currentTime = 0.f;
    m_hasFinished = false;
}

void Animation::play() {
    m_isPlaying = true;
    if (m_hasFinished) {
        reset();  // Restart if it had finished
    }
}

void Animation::pause() {
    m_isPlaying = false;
}

void Animation::stop() {
    m_isPlaying = false;
    reset();
}