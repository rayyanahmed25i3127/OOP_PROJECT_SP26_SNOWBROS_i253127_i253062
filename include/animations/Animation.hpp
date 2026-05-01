#pragma once
#include <SFML/Graphics.hpp>

/**
 * @brief Frame-based sprite animation system.
 *
 * Handles sprite sheet animations by cycling through a sequence of frames
 * at a specified frame rate. Supports looping and one-shot animations.
 *
 * Usage:
 *   1. Load a texture with all animation frames
 *   2. Create Animation with frame rectangles and timing
 *   3. Call update(dt) every frame
 *   4. Apply current frame to sprite: sprite.setTextureRect(anim.getCurrentFrame())
 */
class Animation {
private:
    static const int MAX_FRAMES = 32;  // Maximum frames per animation
    
    sf::IntRect m_frames[MAX_FRAMES];  // Frame rectangles in the sprite sheet
    int   m_frameCount;                // Number of frames added
    float m_frameDuration;             // Seconds per frame
    float m_currentTime;               // Time elapsed in current frame
    int   m_currentFrameIndex;         // Which frame we're on (0 to frameCount-1)
    bool  m_isLooping;                 // True = restart at end, False = stop at last frame
    bool  m_isPlaying;                 // True = actively updating, False = paused
    bool  m_hasFinished;               // True when non-looping animation reaches end

public:
    /**
     * @brief Construct an animation.
     * @param frameDuration Seconds to display each frame (e.g., 0.1f = 10 FPS)
     * @param looping True to restart at end, false to stop at last frame
     */
    Animation(float frameDuration = 0.1f, bool looping = true);

    /**
     * @brief Add a frame rectangle to the animation sequence.
     * @param rect The portion of the sprite sheet for this frame (x, y, width, height)
     */
    void addFrame(const sf::IntRect& rect);

    /**
     * @brief Update animation timing.
     * @param dt Delta time in seconds
     */
    void update(float dt);

    /**
     * @brief Get the current frame rectangle to apply to a sprite.
     */
    sf::IntRect getCurrentFrame() const;

    /**
     * @brief Reset animation to first frame.
     */
    void reset();

    /**
     * @brief Start/resume animation playback.
     */
    void play();

    /**
     * @brief Pause animation (stays on current frame).
     */
    void pause();

    /**
     * @brief Stop animation and reset to first frame.
     */
    void stop();

    /**
     * @brief Check if animation is currently playing.
     */
    bool isPlaying() const { return m_isPlaying; }

    /**
     * @brief Check if non-looping animation has finished.
     */
    bool hasFinished() const { return m_hasFinished; }

    /**
     * @brief Get total number of frames.
     */
    int getFrameCount() const { return m_frameCount; }

    /**
     * @brief Set whether animation loops.
     */
    void setLooping(bool loop) { m_isLooping = loop; }

    /**
     * @brief Set frame duration (speed).
     */
    void setFrameDuration(float duration) { m_frameDuration = duration; }
};