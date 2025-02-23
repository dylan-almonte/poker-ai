#pragma once

#include <memory>
#include <vector>
#include "deck.hpp"
#include "card.hpp"

class PokerTable;
class Player;

class Dealer {
public:
    Dealer();

    // Deal methods
    std::shared_ptr<Card> dealCard();
    void dealPrivateCards(const std::vector<std::shared_ptr<Player>>& players);
    void dealCommunityCards(std::shared_ptr<PokerTable> table, int num_cards);
    
    // Convenience methods for specific rounds
    void dealFlop(std::shared_ptr<PokerTable> table);
    void dealTurn(std::shared_ptr<PokerTable> table);
    void dealRiver(std::shared_ptr<PokerTable> table);

private:
    std::unique_ptr<Deck> deck_;
}; 