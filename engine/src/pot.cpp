#include "pot.hpp"
#include "player.hpp"
#include <algorithm>
#include <random>

namespace {
    std::string generateUuid() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* digits = "0123456789abcdef";

        std::string uuid;
        for (int i = 0; i < 32; ++i) {
            uuid += digits[dis(gen)];
        }
        return uuid;
    }
}

Pot::Pot() : uid_(generateUuid()) {}

void Pot::addChips(Player* player, int chips) {
    if (chips < 0) {
        throw std::invalid_argument("Cannot add negative chips to pot");
    }
    pot_[player] += chips;
}

void Pot::reset() {
    pot_.clear();
}

int Pot::getPlayerContribution(const Player* player) const {
    auto it = pot_.find(const_cast<Player*>(player));
    return it != pot_.end() ? it->second : 0;
}

int Pot::getTotal() const {
    int total = 0;
    for (const auto& [player, chips] : pot_) {
        total += chips;
    }
    return total;
}

std::vector<std::unordered_map<Player*, int>> Pot::getSidePots() const {
    std::vector<std::unordered_map<Player*, int>> side_pots;
    
    if (pot_.empty()) {
        return side_pots;
    }

    // Create a copy of the pot to work with
    auto remaining_chips = pot_;

    while (!remaining_chips.empty()) {
        // Find minimum bet among remaining players
        int min_bet = std::numeric_limits<int>::max();
        for (const auto& [player, chips] : remaining_chips) {
            if (chips > 0) {
                min_bet = std::min(min_bet, chips);
            }
        }

        // Create new side pot
        side_pots.emplace_back();
        auto& current_pot = side_pots.back();

        // Remove min_bet from each player and add to side pot
        std::vector<Player*> players_to_remove;
        for (auto& [player, chips] : remaining_chips) {
            current_pot[player] = min_bet;
            chips -= min_bet;
            if (chips == 0) {
                players_to_remove.push_back(player);
            }
        }

        // Remove players with no chips left
        for (auto* player : players_to_remove) {
            remaining_chips.erase(player);
        }
    }

    return side_pots;
} 