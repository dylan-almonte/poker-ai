#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

/**
 * @brief Represents a playing card in a poker game.
 * 
 * A card is represented internally as a 32-bit integer where:
 * - Bits 16-28: Bit rank (1 bit per rank)
 * - Bits 12-15: Suit
 * - Bits 8-11: Rank
 * - Bits 0-7: Prime number for the rank
 */
class Card {
private:
    int card_int_;                                    // Internal 32-bit integer representation
    static const std::string INT_SUIT_TO_CHAR_SUIT;  // Mapping from suit integer to character
    static const std::unordered_map<int, std::string> PRETTY_SUITS;  // Pretty suit symbols

public:
    static const std::unordered_map<char, int> CHAR_SUIT_TO_INT_SUIT;  // Mapping from suit character to integer
    static const std::unordered_map<char, int> CHAR_RANK_TO_INT_RANK;  // Mapping from rank character to integer
    static const std::string STR_RANKS;                                // String of all possible ranks
    static const std::array<int, 13> PRIMES;                          // Prime numbers for each rank

    /**
     * @brief Constructs a card from a string representation.
     * 
     * @param card_string String in format "rank"suit" (e.g., "As" for Ace of Spades)
     */
    Card(const std::string& card_string);

    /**
     * @brief Constructs a card from an integer representation.
     * 
     * @param card_int Internal 32-bit integer representation
     */
    Card(int card_int);

    /**
     * @brief Creates a card from a string representation.
     * 
     * @param card_string String in format "rank"suit" (e.g., "As" for Ace of Spades)
     * @return A new Card instance
     */
    static Card fromString(const std::string& card_string);

    /**
     * @brief Creates a card from an integer representation.
     * 
     * @param card_int Internal 32-bit integer representation
     * @return A new Card instance
     */
    static Card fromInt(int card_int);

    /**
     * @brief Gets the rank of the card (0-12, where 0 is 2 and 12 is Ace).
     * 
     * @return The rank of the card
     */
    int getRank() const;

    /**
     * @brief Gets the suit of the card (1=spades, 2=hearts, 4=diamonds, 8=clubs).
     * 
     * @return The suit of the card
     */
    int getSuit() const;

    /**
     * @brief Gets the bit rank of the card (1 bit per rank).
     * 
     * @return The bit rank of the card
     */
    int getBitRank() const;

    /**
     * @brief Gets the prime number associated with the card's rank.
     * 
     * @return The prime number for the rank
     */
    int getPrime() const;

    /**
     * @brief Gets the internal integer representation of the card.
     * 
     * @return The internal integer representation
     */
    int toInt() const { return card_int_; }

    /**
     * @brief Converts the card to a string representation.
     * 
     * @return String in format "rank"suit" (e.g., "As" for Ace of Spades)
     */
    std::string toString() const;

    /**
     * @brief Converts the card to a pretty string representation with suit symbols.
     * 
     * @return String in format "[ rank ♠ ]" (e.g., "[ A ♠ ]" for Ace of Spades)
     */
    std::string prettyString() const;

    /**
     * @brief Converts the card to a binary string representation.
     * 
     * @return String showing the 32-bit binary representation
     */
    std::string binaryString() const;

    /**
     * @brief Checks if this card is equal to another card.
     * 
     * @param other The other card to compare with
     * @return true if the cards are equal, false otherwise
     */
    bool operator==(const Card& other) const { return card_int_ == other.card_int_; }

    /**
     * @brief Checks if this card is not equal to another card.
     * 
     * @param other The other card to compare with
     * @return true if the cards are not equal, false otherwise
     */
    bool operator!=(const Card& other) const { return !(*this == other); }
};

/**
 * @brief Creates a vector of cards from a vector of string representations.
 * 
 * @param card_strs Vector of card strings
 * @return Vector of Card objects
 */
std::vector<Card> cardsFromStrings(const std::vector<std::string>& card_strs);

/**
 * @brief Calculates the product of prime numbers for a hand of cards.
 * 
 * @param cards Vector of cards
 * @return Product of prime numbers
 */
int primeProductFromHand(const std::vector<Card>& cards);

/**
 * @brief Calculates the product of prime numbers from a bit rank.
 * 
 * @param rankbits Bit rank representation
 * @return Product of prime numbers
 */
int primeProductFromRankbits(int rankbits);

/**
 * @brief Creates a pretty string representation of a vector of cards.
 * 
 * @param cards Vector of cards
 * @return Pretty string representation
 */
std::string prettyPrintCards(const std::vector<Card>& cards); 