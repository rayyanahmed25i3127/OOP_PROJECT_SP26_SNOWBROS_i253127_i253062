#pragma once
#include "LeaderboardEntry.hpp"
#include <string>


class Leaderboard {
private:
    static const int MAX_ENTRIES = 100;

    LeaderboardEntry m_entries[MAX_ENTRIES];
    int m_count;

public:
    Leaderboard();

    void loadFromFile(const std::string& filename);

    const LeaderboardEntry* getEntries() const;
    int getCount() const;
};