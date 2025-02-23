#include "deck.hpp"
#include <algorithm>
#include <random>
#include <stdexcept>

Deck::Deck(const std::vector<std::string>& include_suits,
           const std::vector<int>& include_ranks)
    : include_suits_(include_suits)
    , include_ranks_(include_ranks) {
    reset();
}

std::shared_ptr<Card> Deck::pick(bool random) {
    if (cards_in_deck_.empty()) {
        throw std::runtime_error("Deck is empty - please use Deck::reset()");
    }

    size_t index;
    if (random) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, cards_in_deck_.size() - 1);
        index = dis(gen);
    } else {
        index = cards_in_deck_.size() - 1;
    }

    auto card = cards_in_deck_[index];
    cards_in_deck_.erase(cards_in_deck_.begin() + index);
    dealt_cards_.push_back(card);
    return card;
}

void Deck::remove(std::shared_ptr<Card> card) {
    auto it = std::find(cards_in_deck_.begin(), cards_in_deck_.end(), card);
    if (it != cards_in_deck_.end()) {
        cards_in_deck_.erase(it);
        dealt_cards_.push_back(card);
    }
}

void Deck::reset() {
    cards_in_deck_.clear();
    dealt_cards_.clear();

    // Create all possible cards from the included suits and ranks
    for (const auto& suit : include_suits_) {
        for (int rank : include_ranks_) {
            cards_in_deck_.push_back(std::make_shared<Card>(rank, suit));
        }
    }

    // Shuffle the deck
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(cards_in_deck_.begin(), cards_in_deck_.end(), gen);
} 