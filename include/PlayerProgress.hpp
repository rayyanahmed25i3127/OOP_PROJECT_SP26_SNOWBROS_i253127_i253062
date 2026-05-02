#pragma once

struct PlayerProgress {
    int gems  = 0;
    int score = 0;

    // -- Shop purchases (applied when PlayState resumes from pause) -----------
    // Items 0-3: bought once per shop visit (reset by clearPending).
    // Extra Life: stackable -- each purchase increments the count.
    bool pendingSpeed          = false;
    bool pendingSnowball       = false;
    bool pendingDistance       = false;
    bool pendingBalloon        = false;
    int  pendingExtraLifeCount = 0;

    // -- Continue (GameOver screen) -------------------------------------------
    // continueCount tracks how many times Continue has been used this session.
    //   cost = 5 + continueCount * 10  =>  5, 15, 25, 35 ...
    // pendingRevive is set by GameOverState so PlayState::onResume() knows to
    // reset m_gameOver, give 1 life, and respawn the player.
    int  continueCount = 0;
    bool pendingRevive = false;

    void clearPending() {
        pendingSpeed          = false;
        pendingSnowball       = false;
        pendingDistance       = false;
        pendingBalloon        = false;
        pendingExtraLifeCount = 0;
        // Note: pendingRevive is cleared by PlayState::onResume, not here.
    }
};