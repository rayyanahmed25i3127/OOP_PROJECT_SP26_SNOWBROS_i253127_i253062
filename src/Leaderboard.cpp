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
    char line[200];

    while (file.getline(line, 200)) {
        if (m_count >= MAX_ENTRIES) break;

        char name[100];
        int score, level;
        char date[20];

        sscanf(line, "%[^,],%d,%d,%s", name, &score, &level, date);

        m_entries[m_count].playerName = name;
        m_entries[m_count].score = score;
        m_entries[m_count].levelReached = level;  // Fixed: actually store level
        m_entries[m_count].date = date;

        m_count++;
    }
   
    // Sort by score descending
    for (int i = 0; i < m_count - 1; i++) {
        for (int j = 0; j < m_count - i - 1; j++) {
            if (m_entries[j].score < m_entries[j + 1].score) {
                LeaderboardEntry temp = m_entries[j];
                m_entries[j] = m_entries[j + 1];
                m_entries[j + 1] = temp;
            }
        }
    }

    // Limit to top 10
    if (m_count > 10) {
        m_count = 10;
    }

    // Write back sorted top 10
    std::ofstream outFile(filename);
    for (int i = 0; i < m_count; i++) {
        // Fixed: use commas and actual level value
        outFile << m_entries[i].playerName << ","
                << m_entries[i].score << ","
                << m_entries[i].levelReached << ","
                << m_entries[i].date << "\n";
    }
    outFile.close();
    file.close();
}

const LeaderboardEntry* Leaderboard::getEntries() const {
    return m_entries;
}

int Leaderboard::getCount() const {
    return m_count;
}