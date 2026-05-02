#pragma once

struct PlayerProgress {
    int gems = 0;
    int score = 0;
    
    // Shop purchases pending application when game resumes from pause.
    // Items 0-3: bought once per shop visit (bool flag).
    // Extra Life: stackable — each purchase increments the count.
    bool pendingSpeed      = false;
    bool pendingSnowball   = false;
    bool pendingDistance   = false;
    bool pendingBalloon    = false;
    int  pendingExtraLifeCount = 0;   // replaces bool — allows buying multiple lives
    
    void clearPending() {
        pendingSpeed             = false;
        pendingSnowball          = false;
        pendingDistance          = false;
        pendingBalloon           = false;
        pendingExtraLifeCount    = 0;
    }
};