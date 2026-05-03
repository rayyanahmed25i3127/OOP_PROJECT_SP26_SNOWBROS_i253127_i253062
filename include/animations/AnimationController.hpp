#pragma once
#include "Animation.hpp"
#include <SFML/Graphics.hpp>
#include <string>

//Yeh ek **Animation Controller** hai jo ek entity (jaise ke game character) 
//ki mukhtalif animations ko handle karta hai. Iska kaam animations ke darmiyan 
//switch karna hai, jaise ke "idle" se "walk" ya "jump" par jana. Update loop ke 
//doran, yeh automatic taur par sahi frame sprite par apply kar deta hai taake 
//character move hota hua dikhayi de.

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
    int m_currentIndex;  // abhi wale frame ka index

public:
    AnimationController();

    // animation with name like walk1.png jump.png etc
    void addAnimation(const std::string& name, const Animation& anim);

    
     //Switch to a different animation by name    
    bool setAnimation(const std::string& name, bool restart = false);
                                        //^^ animation name to switch to
   // update current animation
    void update(float dt);

    // apply curent animation to sprite
    void applyToSprite(sf::Sprite& sprite);

    // get name of currently playing animation
    std::string getCurrentAnimationName() const;

    //check if current animation has finished
    bool hasCurrentAnimationFinished() const;

    // get pointer to animation by name.
    Animation* getAnimation(const std::string& name);
};