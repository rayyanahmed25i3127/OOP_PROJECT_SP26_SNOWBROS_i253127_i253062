#include "physics/CollisionDetector.hpp"
#include <iostream>

CollisionDetector::CollisionDetector(float leftWall, float rightWall)
    : m_leftWall(leftWall), m_rightWall(rightWall)
{}

void CollisionDetector::resolve(Player& player,
                                Platform* const platforms[],
                                int platformCount,
                                float prevY) const
{
    sf::Vector2f pos = player.getPosition();
    sf::Vector2f vel = player.getVelocity();
    sf::FloatRect hit = player.getHitBox();   // position is hitbox's actual top-left

    // Offset of hitbox inside the sprite (computed from actual hitbox vs position)
    float hitOffsetX = hit.position.x - pos.x;
    float hitOffsetY = hit.position.y - pos.y;

    // The hitbox's previous-frame Y (translate from sprite's prevY)
    float hitPrevY = prevY + hitOffsetY;

    // =============================================================
    // 1. HORIZONTAL WALL COLLISION (against the hitbox, not sprite)
    // =============================================================
    // Left wall: if hitbox's left edge goes past m_leftWall, push back.
    // We correct the SPRITE's position since that's what we store.
    if (hit.position.x < m_leftWall) {
        pos.x = m_leftWall - hitOffsetX;
        if (vel.x < 0) vel.x = 0.f;
    }
    // Right wall: hitbox's right edge can't exceed m_rightWall
    float hitRightLimit = m_rightWall - hit.size.x;
    if (hit.position.x > hitRightLimit) {
        pos.x = hitRightLimit - hitOffsetX;
        if (vel.x > 0) vel.x = 0.f;
    }

    // Refresh hit position after horizontal correction
    hit.position.x = pos.x + hitOffsetX;

    // =============================================================
    // 2. ONE-WAY PLATFORM COLLISION
    // =============================================================
    bool landed = false;

    for (int i = 0; i < platformCount; ++i) {
        sf::FloatRect plat = platforms[i]->getBounds();

        float hitBottomNow  = hit.position.y + hit.size.y;
        float hitBottomPrev = hitPrevY        + hit.size.y;

        float hitLeft   = hit.position.x;
        float hitRight  = hit.position.x + hit.size.x;
        float platLeft  = plat.position.x;
        float platRight = plat.position.x + plat.size.x;

        bool horizontalOverlap = (hitRight > platLeft) && (hitLeft < platRight);
        if (!horizontalOverlap) continue;

        float platTop = plat.position.y;
        bool wasAbove = (hitBottomPrev <= platTop);
        bool nowBelow = (hitBottomNow  >= platTop);
        bool falling  = (vel.y >= 0.f);

        if (wasAbove && nowBelow && falling) {
            // Snap hitbox bottom to platform top → translate back to sprite position
            pos.y = platTop - hit.size.y - hitOffsetY;
            vel.y = 0.f;
            landed = true;
        }
    }

    // =============================================================
    // 3. FALLBACK GROUND (bottom of visible play area)
    // =============================================================
    // Hitbox bottom should rest on the snow line.
    const float GROUND_Y_HITBOX_BOTTOM = 570.f;  // adjust to match snow line
    float hitBottom = hit.position.y + hit.size.y;
    if (hitBottom >= GROUND_Y_HITBOX_BOTTOM) {
        pos.y = GROUND_Y_HITBOX_BOTTOM - hit.size.y - hitOffsetY;
        vel.y = 0.f;
        landed = true;
    }

    // Write back
    player.setPosition(pos);
    player.setVelocity(vel);
    player.setOnGround(landed);
}