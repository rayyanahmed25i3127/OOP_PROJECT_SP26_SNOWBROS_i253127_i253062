#pragma once
#include <string>

struct LeaderboardEntry {
    std::string playerName;
    long long score;
    int levelReached;  // Make sure this field exists
    std::string date;
};