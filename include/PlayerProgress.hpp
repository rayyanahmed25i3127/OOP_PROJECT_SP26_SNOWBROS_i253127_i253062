#pragma once

struct PlayerProgress {
    int gems  = 0;
    int score = 0;

    // -- Shop purchases (applied when PlayState resumes from pause) -----------
    bool pendingSpeed          = false;
    bool pendingSnowball       = false;
    bool pendingDistance       = false;
    bool pendingBalloon        = false;
    int  pendingExtraLifeCount = 0;

    // -- GameOver Continue button (GameOver screen) ---------------------------
    // cost = 5 + continueCount * 10  =>  5, 15, 25 ...
    int  continueCount = 0;
    bool pendingRevive = false;

    // -- Main Menu Continue button --------------------------------------------
    // Set by PlayState::onExit() when the player voluntarily quits mid-game.
    // Cleared when the continued game starts (PlayState::onEnter reads these).
    // savedLevel == 0  =>  no active save / nothing to continue.
    // gameOverOccurred =>  game ended naturally; Continue must show error.
    int  savedLevel          = 0;
    int  savedGems           = 0;
    int  savedCharacterIndex = 0;
    bool gameOverOccurred    = false;

    void clearPending() {
        pendingSpeed          = false;
        pendingSnowball       = false;
        pendingDistance       = false;
        pendingBalloon        = false;
        pendingExtraLifeCount = 0;
        // pendingRevive cleared by PlayState::update, not here.
    }

    void clearSave() {
        savedLevel          = 0;
        savedGems           = 0;
        savedCharacterIndex = 0;
        gameOverOccurred    = false;
    }
};