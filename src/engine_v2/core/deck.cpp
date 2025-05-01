#include "deck.hpp"
#include <random>
#include <algorithm>
#include <stdexcept>

// Static member initialization
std::vector<Card> Deck::FULL_DECK;

/**
 * @brief Constructs a new deck of cards.
 * 
 * Creates a full deck of 52 cards and shuffles them.
 */
Deck::Deck() {
    cards_ = getFullDeck();
    shuffle();
}

/**
 * @brief Creates a full deck of 52 cards.
 * 
 * @return Vector containing all 52 cards in a standard deck
 */
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

/**
 * @brief Shuffles the deck of cards.
 * 
 * Uses the Mersenne Twister random number generator to randomly reorder the cards.
 */
void Deck::shuffle() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(cards_.begin(), cards_.end(), gen);
}

/**
 * @brief Draws cards from the deck.
 * 
 * @param num Number of cards to draw (default: 1)
 * @return Vector of drawn cards
 * @throws std::runtime_error if trying to draw more cards than available
 */
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

/**
 * @brief Converts the deck to a string representation.
 * 
 * @return Pretty string representation of all cards in the deck
 */
std::string Deck::toString() const {
    return prettyPrintCards(cards_);
} 