#include "physics/CollisionDetector.hpp"
#include <iostream>

CollisionDetector::CollisionDetector(float leftWall, float rightWall)
    : m_leftWall(leftWall), m_rightWall(rightWall)
{}

void CollisionDetector::resolve(Entity& entity,
                                Platform* const platforms[],
                                int platformCount,
                                float prevX,
                                float prevY) const
{
    sf::Vector2f pos  = entity.getPosition();
    sf::Vector2f vel  = entity.getVelocity();
    sf::FloatRect hit = entity.getHitBox();

    float hitOffsetX = hit.position.x - pos.x;
    float hitOffsetY = hit.position.y - pos.y;

    float hitPrevX = prevX + hitOffsetX;
    float hitPrevY = prevY + hitOffsetY;

    // =============================================================
    // 1. HORIZONTAL WALL COLLISION (screen borders)
    // =============================================================
    if (hit.position.x < m_leftWall) {
        pos.x = m_leftWall - hitOffsetX;
        if (vel.x < 0) vel.x = 0.f;
    }
    float hitRightLimit = m_rightWall - hit.size.x;
    if (hit.position.x > hitRightLimit) {
        pos.x = hitRightLimit - hitOffsetX;
        if (vel.x > 0) vel.x = 0.f;
    }
    hit.position.x = pos.x + hitOffsetX;

    // =============================================================
    // 2. SOLID HIT-BOX HORIZONTAL COLLISION
    // =============================================================
    for (int i = 0; i < platformCount; ++i) {
        if (!platforms[i]) continue;
        int boxCount = platforms[i]->getHitboxCount();
        for (int b = 0; b < boxCount; ++b) {
            if (!platforms[i]->isSolid(b)) continue;

            sf::FloatRect plat = platforms[i]->getBounds(b);

            float hitLeft   = hit.position.x;
            float hitRight  = hit.position.x + hit.size.x;
            float hitTop    = hit.position.y;
            float hitBottom = hit.position.y + hit.size.y;
            float platLeft   = plat.position.x;
            float platRight  = plat.position.x + plat.size.x;
            float platTop    = plat.position.y;
            float platBottom = plat.position.y + plat.size.y;

            bool overlapping =
                (hitRight > platLeft) && (hitLeft < platRight) &&
                (hitBottom > platTop) && (hitTop < platBottom);
            if (!overlapping) continue;

            float hitPrevTop    = hitPrevY;
            float hitPrevBottom = hitPrevY + hit.size.y;

            bool wasVertOverlap =
                (hitPrevBottom > platTop) && (hitPrevTop < platBottom);
            bool wasStandingOnTop =
                (hitPrevBottom <= platTop + 1.f) &&
                (hitPrevBottom >= platTop - 1.f);

            if (!wasVertOverlap && !wasStandingOnTop) continue;

            float hitPrevLeft  = hitPrevX;
            float hitPrevRight = hitPrevX + hit.size.x;

            if (hitPrevRight <= platLeft) {
                pos.x = platLeft - hit.size.x - hitOffsetX;
                if (vel.x > 0) vel.x = 0.f;
            } else if (hitPrevLeft >= platRight) {
                pos.x = platRight - hitOffsetX;
                if (vel.x < 0) vel.x = 0.f;
            }
            hit.position.x = pos.x + hitOffsetX;
        }
    }

    // =============================================================
    // 3. ONE-WAY PLATFORM LANDING (from above, when falling)
    // =============================================================
    bool landed = false;
    for (int i = 0; i < platformCount; ++i) {
        if (!platforms[i]) continue;
        int boxCount = platforms[i]->getHitboxCount();
        for (int b = 0; b < boxCount; ++b) {
            sf::FloatRect plat = platforms[i]->getBounds(b);

            float hitBottomNow  = hit.position.y + hit.size.y;
            float hitBottomPrev = hitPrevY + hit.size.y;
            float hitLeft   = hit.position.x;
            float hitRight  = hit.position.x + hit.size.x;
            float platLeft  = plat.position.x;
            float platRight = plat.position.x + plat.size.x;

            bool horizontalOverlap =
                (hitRight > platLeft) && (hitLeft < platRight);
            if (!horizontalOverlap) continue;

            float platTop = plat.position.y;
            bool wasAbove = (hitBottomPrev <= platTop);
            bool nowBelow = (hitBottomNow  >= platTop);
            bool falling  = (vel.y >= 0.f);

            if (wasAbove && nowBelow && falling) {
                pos.y = platTop - hit.size.y - hitOffsetY;
                vel.y = 0.f;
                landed = true;
            }
        }
    }

    // =============================================================
    // 4. FALLBACK GROUND
    // =============================================================
    const float GROUND_Y_HITBOX_BOTTOM = 570.f;
    float hitBottom = hit.position.y + hit.size.y;
    if (hitBottom >= GROUND_Y_HITBOX_BOTTOM) {
        pos.y = GROUND_Y_HITBOX_BOTTOM - hit.size.y - hitOffsetY;
        vel.y = 0.f;
        landed = true;
    }

    entity.setPosition(pos);
    entity.setVelocity(vel);
    entity.setOnGround(landed);
}

bool CollisionDetector::checkEnemyContact(const Entity& player,
                                          Enemy* const enemies[],
                                          int enemyCount) const
{
    sf::FloatRect playerHit = player.getHitBox();
    float pLeft   = playerHit.position.x;
    float pRight  = playerHit.position.x + playerHit.size.x;
    float pTop    = playerHit.position.y;
    float pBottom = playerHit.position.y + playerHit.size.y;

    for (int i = 0; i < enemyCount; ++i) {
        if (!enemies[i]) continue;
        // Snowballed/Rolling/Dead enemies don't kill the player.
        // Only Alive enemies are lethal on contact.
        if (enemies[i]->getState() != Enemy::State::Alive) continue;

        sf::FloatRect eHit = enemies[i]->getHitBox();
        float eLeft   = eHit.position.x;
        float eRight  = eHit.position.x + eHit.size.x;
        float eTop    = eHit.position.y;
        float eBottom = eHit.position.y + eHit.size.y;

        bool overlap =
            (pRight > eLeft) && (pLeft < eRight) &&
            (pBottom > eTop) && (pTop < eBottom);

        if (overlap) return true;
    }
    return false;
}