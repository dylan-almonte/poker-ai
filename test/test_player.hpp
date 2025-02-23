#pragma once
#include "player.hpp"

// Create a simple test player that always calls
class TestPlayer : public Player {
public:
    TestPlayer(const std::string& name, int chips, std::shared_ptr<Pot> pot)
        : Player(name, chips, pot) {}

    std::unique_ptr<Action> takeAction(void* game_state) override {
        return call(std::vector<std::shared_ptr<Player>>()); // Always call
    }
}; 