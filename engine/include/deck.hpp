#pragma once
#include "card.hpp"
#include <vector>
#include <memory>

class Deck {
private:
    std::vector<Card> cards;
    static std::vector<Card> FULL_DECK;
    static std::vector<Card> getFullDeck();

public:
    Deck();
    void shuffle();
    std::vector<Card> draw(size_t num = 1);
    std::string toString() const;
    size_t size() const { return cards.size(); }
}; 