#pragma once

#include <memory>
#include <vector>
#include <string>
#include "card.hpp"

class Deck {
public:
    Deck(const std::vector<std::string>& include_suits = Card::getAllSuits(),
         const std::vector<int>& include_ranks = {2,3,4,5,6,7,8,9,10,11,12,13,14});

    // Card operations
    std::shared_ptr<Card> pick(bool random = true);
    void remove(std::shared_ptr<Card> card);
    void reset();

    // Getters
    size_t size() const { return cards_in_deck_.size() + dealt_cards_.size(); }

private:
    std::vector<std::shared_ptr<Card>> cards_in_deck_;
    std::vector<std::shared_ptr<Card>> dealt_cards_;
    std::vector<std::string> include_suits_;
    std::vector<int> include_ranks_;
}; 