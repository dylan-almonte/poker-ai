#include <gtest/gtest.h>
#include "engine.hpp"
#include "player.hpp"
#include "table.hpp"
#include "pot.hpp"
#include "test_player.hpp"

class PokerEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        pot = std::make_shared<Pot>();
        players.clear();
        for (int i = 0; i < 6; i++) {
            players.push_back(std::make_shared<TestPlayer>(
                "Player " + std::to_string(i), 1000, pot));
        }
        table = std::make_shared<PokerTable>(players, pot);
        engine = std::make_unique<PokerEngine>(table, 10, 20);
    }

    std::shared_ptr<Pot> pot;
    std::vector<std::shared_ptr<Player>> players;
    std::shared_ptr<PokerTable> table;
    std::unique_ptr<PokerEngine> engine;
};

TEST_F(PokerEngineTest, BlindPostingWorks) {
    engine->roundSetup();
    EXPECT_EQ(players[0]->getBetChips(), 10); // Small blind
    EXPECT_EQ(players[1]->getBetChips(), 20); // Big blind
}

TEST_F(PokerEngineTest, ChipsAreConserved) {
    int initial_total = table->getTotalChips();
    engine->playOneRound();
    EXPECT_EQ(table->getTotalChips(), initial_total);
}

// Add more tests...

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 