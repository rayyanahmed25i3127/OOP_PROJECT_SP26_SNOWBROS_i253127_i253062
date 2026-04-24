#include "Leaderboard.hpp"
#include <fstream>
#include <iostream>

Leaderboard::Leaderboard() : m_count(0) {}

void Leaderboard::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open leaderboard file\n";
        return;
    }

    m_count = 0;

    std::string name, date;
    int score, level;

    while (file >> name >> score >> level >> date) {
        if (m_count >= MAX_ENTRIES) break;

        m_entries[m_count].playerName = name;
        m_entries[m_count].score = score;
        m_entries[m_count].date = date;

        m_count++;
    }

    file.close();
}

const LeaderboardEntry* Leaderboard::getEntries() const {
    return m_entries;
}

int Leaderboard::getCount() const {
    return m_count;
}