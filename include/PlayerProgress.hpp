#pragma once
#include <string>

struct PlayerProgress {
    // -- Multiplayer mode flag --
    bool        isMultiplayer  = false;
    std::string player2Name;          // entered by user before MP game starts

    int gems  = 0;
    int score = 0;

    // -- Shop purchases (applied when PlayState resumes from pause) -----------
    bool pendingSpeed          = false;
    bool pendingSnowball       = false;
    bool pendingDistance       = false;
    bool pendingBalloon        = false;
    int  pendingExtraLifeCount = 0;

    // -- Per-player shop purchases (multiplayer) --
    bool pendingSpeedP2          = false;
    bool pendingSnowballP2       = false;
    bool pendingDistanceP2       = false;
    bool pendingBalloonP2        = false;
    int  pendingExtraLifeCountP2 = 0;
    int  gemsP1 = 0;   // per-player diamond tracking in multiplayer
    int  gemsP2 = 0;

    // -- GameOver Continue button (GameOver screen) ---------------------------
    int  continueCount = 0;
    bool pendingRevive = false;

    // -- Main Menu Continue button --------------------------------------------
    int  savedLevel          = 0;
    int  savedGems           = 0;
    int  savedCharacterIndex = 0;
    bool gameOverOccurred    = false;

    void clearPendingP2() {
        pendingSpeedP2          = false;
        pendingSnowballP2       = false;
        pendingDistanceP2       = false;
        pendingBalloonP2        = false;
        pendingExtraLifeCountP2 = 0;
    }

    void clearPending() {
        pendingSpeed          = false;
        pendingSnowball       = false;
        pendingDistance       = false;
        pendingBalloon        = false;
        pendingExtraLifeCount = 0;
        clearPendingP2();
    }

    void clearSave() {
        savedLevel          = 0;
        savedGems           = 0;
        savedCharacterIndex = 0;
        gameOverOccurred    = false;
    }
};