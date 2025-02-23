#include "table.hpp"
#include <numeric>
#include <stdexcept>

PokerTable::PokerTable(const std::vector<std::shared_ptr<Player>>& players,
                       std::shared_ptr<Pot> pot)
    : players_(players)
    , pot_(pot)
    , dealer_(std::make_shared<Dealer>()) {
    
    if (players_.size() < 2) {
        throw std::runtime_error("Must have at least 2 players at the table");
    }

    // Verify all players point to same pot
    for (const auto& player : players_) {
        if (player->getPot() != pot_) {
            throw std::runtime_error("All players must share the same pot");
        }
    }
}

void PokerTable::setPlayers(const std::vector<std::shared_ptr<Player>>& players) {
    players_ = players;
    for (const auto& player : players_) {
        if (player->getPot() != pot_) {
            throw std::runtime_error("All players must share the same pot");
        }
    }
}

void PokerTable::addCommunityCard(std::shared_ptr<Card> card) {
    community_cards_.push_back(card);
}

int PokerTable::getTotalChips() const {
    return std::accumulate(players_.begin(), players_.end(), 0,
        [](int sum, const auto& player) { 
            return sum + player->getChips(); 
        });
} 