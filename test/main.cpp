#include <iostream>
#include <memory>
#include <vector>
#include "engine.hpp"
#include "player.hpp"
#include "table.hpp"
#include "pot.hpp"
#include <cassert>
#include "test_player.hpp"

void printPlayerStatus(const std::shared_ptr<Player>& player) {
    std::cout << player->getName() << ": " 
              << "Chips=" << player->getChips() 
              << " Active=" << player->isActive()
              << " AllIn=" << player->isAllIn() << std::endl;
}

void testSpecificScenario() {
    auto pot = std::make_shared<Pot>();
    std::vector<std::shared_ptr<Player>> players;
    
    // Create 3 players with specific cards
    auto player1 = std::make_shared<TestPlayer>("Player 1", 1000, pot);
    auto player2 = std::make_shared<TestPlayer>("Player 2", 1000, pot);
    auto player3 = std::make_shared<TestPlayer>("Player 3", 1000, pot);
    
    players.push_back(player1);
    players.push_back(player2);
    players.push_back(player3);

    auto table = std::make_shared<PokerTable>(players, pot);
    PokerEngine engine(table, 10, 20);

    // Force specific cards (you'll need to add methods to set cards)
    player1->addPrivateCard(std::make_shared<Card>("ace", "spades"));
    player2->addPrivateCard(std::make_shared<Card>("king", "spades"));
    player3->addPrivateCard(std::make_shared<Card>("queen", "spades"));

    std::cout << "\nTesting specific scenario:" << std::endl;
    engine.playOneRound();

    // Add assertions to verify expected behavior
    assert(player1->getChips() > 1000); // Winner should have more chips
    assert(player2->getChips() < 1000); // Losers should have fewer chips
    assert(player3->getChips() < 1000);
}

int main() {
    try {
        // Create pot and players
        auto pot = std::make_shared<Pot>();
        std::vector<std::shared_ptr<Player>> players;
        
        // Create 6 players with 1000 chips each
        for (int i = 0; i < 6; i++) {
            players.push_back(std::make_shared<TestPlayer>(
                "Player " + std::to_string(i), 1000, pot));
        }

        // Create table and engine
        auto table = std::make_shared<PokerTable>(players, pot);
        PokerEngine engine(table, 10, 20); // Small blind 10, big blind 20

        std::cout << "Initial state:" << std::endl;
        for (const auto& player : players) {
            printPlayerStatus(player);
        }

        // Play 3 rounds
        for (int round = 0; round < 3; round++) {
            std::cout << "\nPlaying round " << round + 1 << std::endl;
            engine.playOneRound();

            std::cout << "Round " << round + 1 << " results:" << std::endl;
            for (const auto& player : players) {
                printPlayerStatus(player);
            }
        }

        // Add new specific scenario test
        testSpecificScenario();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 