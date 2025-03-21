#include "deck.hpp"
#include <random>
#include <algorithm>
#include <stdexcept>

std::vector<Card> Deck::FULL_DECK;

Deck::Deck() {
    cards_ = getFullDeck();
    shuffle();
}

std::vector<Card> Deck::getFullDeck() {
    if (FULL_DECK.empty()) {
        // Create the standard 52 card deck
        for (char rank : Card::STR_RANKS) {
            for (const auto& [suit, _] : Card::CHAR_SUIT_TO_INT_SUIT) {
                std::string card_str;
                card_str += rank;
                card_str += suit;
                FULL_DECK.push_back(Card(card_str));
            }
        }
    }
    return FULL_DECK;
}

void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(cards_.begin(), cards_.end(), gen);
}

std::vector<Card> Deck::draw(size_t num) {
    if (cards_.size() < num) {
        throw std::runtime_error(
            "Cannot draw " + std::to_string(num) + 
            " cards from deck of size " + std::to_string(cards_.size())
        );
    }

    std::vector<Card> drawn_cards(cards_.begin(), cards_.begin() + num);
    cards_.erase(cards_.begin(), cards_.begin() + num);
    return drawn_cards;
}

std::string Deck::toString() const {
    return prettyPrintCards(cards_);
} 