#pragma once

#include "Entity.hpp"
#include "Platform.hpp"
#include "enemies/Enemy.hpp"
#include <SFML/Graphics.hpp>

//its a central utility class jo Player aur enemies ke liye ek hi tarah se kaam karti hai taake code clean rahe. 
//Iska kaam screen ki boundaries, platofrms hits, aur platforms par landing ko resolve karna hai taake koi bhi object 
//game ki screen se bahar na nikal sake ya zameen ke andar na ghuss jaye.
class CollisionDetector {
private:
    float m_leftWall;
    float m_rightWall;

public:
    CollisionDetector(float leftWall = 30.f, float rightWall = 770.f);

    //Yeh function entity ki movement ke baad platform se takrane ka track
    //rkhta hai aur uski position aur onground status ko sahi karta hai.
    void resolve(Entity& entity,
                 Platform* const platforms[], int platformCount,
                 float prevX, float prevY) const;

    //Yeh function check karta hai ke kya player kisi alive enemy ko hit kia, 
    //aur pehla hit receive hote hi result de deta hai. Yeh sirf un enemies ko dekhta hai 
    //jo "Alive" hon, yani snowball encased ko ignore kar deta hai taake woh player 
    //ke liye dangerous na banein.
    bool checkEnemyContact(const Entity& player,
                           Enemy* const enemies[], int enemyCount) const;

    float getLeftWall()  const { return m_leftWall; }
    float getRightWall() const { return m_rightWall; }
};