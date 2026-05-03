#pragma once
#include <SFML/Graphics.hpp>

/* 
what does this file do ?
yeh file sprites k animations ko handle karti hai like player blue 
k 8-10 frames they in total, unko kis tarah se kaam karvaana hai wo 
kaam yeh file karti hai, aik specific frame rate k according animations 
cycle hoti hain.

 */
class Animation {
private:
    static const int MAX_FRAMES = 32;  // max frames per animation
    sf::IntRect m_frames[MAX_FRAMES];  // Frame rectangles in the sprite sheet
    int   m_frameCount;                // Number of frames added, like jese hamne player blue k liay 8 kiay thy.
    float m_frameDuration;        //fps wagera
    float m_currentTime;               // aik frame pe how much time does it take
    int   m_currentFrameIndex; // checks for the number of frame thats is currently being played
    bool  m_isLooping;                 // for true = restart at end...  False = stop at last frame
    bool  m_isPlaying;                 //true = actively updatingand False = paused
    bool  m_hasFinished;               // yeh sirf tab true hoga jab static sprites honge

public:
    
    Animation(float frameDuration = 0.1f, bool looping = true);
                       // ^^ time for each animation to be displayed 
    void addFrame(const sf::IntRect& rect);
                                       // ^^ portion of sprite ( matlab x,y coordinates k accoridng) 
    void update(float dt); // dt is time in delta, ised for updating frames

    sf::IntRect getCurrentFrame() const; // current frame lo, or uske accoridng add anim

    //reset animation to first frame.
    
    void reset();

    //start/resume animation 
    void play();

    
    //pause animation (stay on current frame)
    void pause();

    //stop animation and reset to first fr
    void stop();

    // check k agar animation abhi play hora ya nai
    bool isPlaying() const { return m_isPlaying; }

    
    // Check if non-looping animation has finished ornot
    
    bool hasFinished() const { return m_hasFinished; }

    //total number of frames
    int getFrameCount() const { return m_frameCount; }

    
//if animation loops or not wala check
     
    void setLooping(bool loop) { m_isLooping = loop; }
// frame duration
    void setFrameDuration(float duration) { m_frameDuration = duration; }
};