#include "Game.hpp"

int main() {
    Game game;
    game.run();
    return 0;
}
//REmoving the test codes of leaderboard
/*
#include <iostream>
#include "Leaderboard.hpp"

int main() {
    Leaderboard lb;

    // Create entries
    LeaderboardEntry e1 = {"Anas", 1200, 2, "2026-04-23"};
    LeaderboardEntry e2 = {"Ali", 900, 1, "2026-04-23"};
    LeaderboardEntry e3 = {"Ahmed", 1500, 3, "2026-04-23"};

    // Add entries
    lb.addEntry(e1);
    lb.addEntry(e2);
    lb.addEntry(e3);

    // Sort
    lb.sortByScore();

    // Save to file
    lb.saveToFile("leaderboard.txt");

    std::cout << "Saved leaderboard!\n";

    // Load again
    Leaderboard lb2;
    lb2.loadFromFile("leaderboard.txt");

    std::cout << "Loaded leaderboard:\n";

    for (const auto& e : lb2.getEntries()) {
        std::cout << e.playerName << " | "
                  << e.score << " | "
                  << e.levelReached << " | "
                  << e.date << "\n";
    }

    return 0;
}*/