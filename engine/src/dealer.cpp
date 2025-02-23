#include "dealer.hpp"
#include "table.hpp"
#include "player.hpp"
#include <stdexcept>

Dealer::Dealer() 
    : deck_(std::make_unique<Deck>()) {
}

std::shared_ptr<Card> Dealer::dealCard() {
    return deck_->pick(true);
}

void Dealer::dealPrivateCards(const std::vector<std::shared_ptr<Player>>& players) {
    // Deal two cards to each player
    for (int i = 0; i < 2; ++i) {
        for (auto& player : players) {
            player->addPrivateCard(dealCard());
        }
    }
}

void Dealer::dealCommunityCards(std::shared_ptr<PokerTable> table, int num_cards) {
    if (num_cards <= 0) {
        throw std::invalid_argument("Number of cards must be positive");
    }
    
    for (int i = 0; i < num_cards; ++i) {
        table->addCommunityCard(dealCard());
    }
}

void Dealer::dealFlop(std::shared_ptr<PokerTable> table) {
    dealCommunityCards(table, 3);
}

void Dealer::dealTurn(std::shared_ptr<PokerTable> table) {
    dealCommunityCards(table, 1);
}

void Dealer::dealRiver(std::shared_ptr<PokerTable> table) {
    dealCommunityCards(table, 1);
} 