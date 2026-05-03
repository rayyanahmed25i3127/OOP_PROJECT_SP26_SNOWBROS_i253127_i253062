#pragma once
#include <string>

struct LeaderboardEntry {
    std::string playerName;
    long long score;
    int levelReached;  
    std::string date;
};