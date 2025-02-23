#pragma once

#include <memory>
#include <vector>
#include "player.hpp"
#include "dealer.hpp"
#include "pot.hpp"
#include "card.hpp"

class PokerTable {
public:
    PokerTable(const std::vector<std::shared_ptr<Player>>& players, 
               std::shared_ptr<Pot> pot);

    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    void setPlayers(const std::vector<std::shared_ptr<Player>>& players);
    
    std::shared_ptr<Dealer> getDealer() const { return dealer_; }
    std::shared_ptr<Pot> getPot() const { return pot_; }
    
    void addCommunityCard(std::shared_ptr<Card> card);
    const std::vector<std::shared_ptr<Card>>& getCommunityCards() const { return community_cards_; }

    int getTotalChips() const;
    size_t getNumPlayers() const { return players_.size(); }

private:
    std::vector<std::shared_ptr<Player>> players_;
    std::shared_ptr<Pot> pot_;
    std::shared_ptr<Dealer> dealer_;
    std::vector<std::shared_ptr<Card>> community_cards_;
    int num_games_{0};
}; 