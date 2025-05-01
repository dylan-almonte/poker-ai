#pragma once
#include "card.hpp"
#include <vector>
#include <memory>

/**
 * @brief Represents a deck of playing cards in a poker game.
 * 
 * A deck contains a collection of cards that can be shuffled and drawn from.
 * The deck maintains its own state and can be used to deal cards to players.
 */
class Deck {
private:
    std::vector<Card> cards_;                    // The current cards in the deck
    static std::vector<Card> FULL_DECK;          // Static full deck of 52 cards
    static std::vector<Card> getFullDeck();      // Helper to create a full deck

public:
    /**
     * @brief Constructs a new deck of cards.
     * 
     * Creates a full deck of 52 cards and shuffles them.
     */
    Deck();

    /**
     * @brief Shuffles the deck of cards.
     * 
     * Randomly reorders the cards in the deck.
     */
    void shuffle();

    /**
     * @brief Draws cards from the deck.
     * 
     * @param num Number of cards to draw (default: 1)
     * @return Vector of drawn cards
     * @throws std::runtime_error if trying to draw more cards than available
     */
    std::vector<Card> draw(size_t num = 1);

    /**
     * @brief Converts the deck to a string representation.
     * 
     * @return Pretty string representation of all cards in the deck
     */
    std::string toString() const;

    /**
     * @brief Gets the number of cards remaining in the deck.
     * 
     * @return Number of cards in the deck
     */
    size_t size() const { return cards_.size(); }
}; 