#pragma once
#include "card.hpp"
#include <vector>
#include <unordered_map>
#include <array>

/**
 * @brief Lookup table for poker hand evaluation.
 * 
 * Contains pre-computed tables for evaluating poker hands efficiently.
 * The tables map prime products to hand ranks for different hand types.
 */
class LookupTable {
public:
    // Maximum ranks for each hand type
    static const int MAX_STRAIGHT_FLUSH = 10;    // Maximum rank for straight flush
    static const int MAX_FOUR_OF_A_KIND = 166;   // Maximum rank for four of a kind
    static const int MAX_FULL_HOUSE = 322;       // Maximum rank for full house
    static const int MAX_FLUSH = 1599;           // Maximum rank for flush
    static const int MAX_STRAIGHT = 1609;        // Maximum rank for straight
    static const int MAX_THREE_OF_A_KIND = 2467; // Maximum rank for three of a kind
    static const int MAX_TWO_PAIR = 3325;        // Maximum rank for two pair
    static const int MAX_PAIR = 6185;            // Maximum rank for pair
    static const int MAX_HIGH_CARD = 7462;       // Maximum rank for high card

    static const std::unordered_map<int, int> MAX_TO_RANK_CLASS;        // Maps max ranks to rank classes
    static const std::unordered_map<int, std::string> RANK_CLASS_TO_STRING;  // Maps rank classes to strings

    /**
     * @brief Constructs a new lookup table.
     * 
     * Initializes the lookup tables for all hand types.
     */
    LookupTable();

private:
    /**
     * @brief Initializes the flush lookup table.
     */
    void _flushes();

    /**
     * @brief Initializes the lookup table for multiple cards of same rank.
     */
    void _multiples();

    /**
     * @brief Initializes the lookup tables for straights and high cards.
     * 
     * @param straights Vector of straight bit patterns
     * @param highcards Vector of high card bit patterns
     */
    void _straight_and_highcards(const std::vector<int>& straights, const std::vector<int>& highcards);

    /**
     * @brief Generates lexicographically next bit sequence.
     * 
     * @param bits Starting bit sequence
     * @return Vector of next bit sequences
     */
    static std::vector<int> _get_lexographically_next_bit_sequence(int bits);
};

/**
 * @brief Evaluates poker hands.
 * 
 * Provides functionality to evaluate poker hands and determine their strength.
 * Uses pre-calculated lookup tables and bitwise operations for efficient evaluation.
 */
class Evaluator {
private:
    // Pre-calculated lookup tables
    static std::array<int, 0x1FFF> flush_lookup_;     // Lookup table for flush hands
    static std::array<int, 0x1FFF> unsuited_lookup_;  // Lookup table for non-flush hands

    /**
     * @brief Fast 7-card evaluation using bitwise operations.
     * 
     * @param cards Vector of 7 cards to evaluate
     * @return Hand rank (lower is better)
     */
    static int evaluate7(const std::vector<Card>& cards);

public:
    /**
     * @brief Evaluates a poker hand against a board.
     * 
     * @param cards Player's hole cards
     * @param board Community cards
     * @return Hand rank (lower is better)
     */
    static int evaluate(const std::vector<Card>& cards, const std::vector<Card>& board);

    /**
     * @brief Gets the rank class of a hand rank.
     * 
     * @param hand_rank Hand rank to classify
     * @return Rank class (1=straight flush, 2=four of a kind, etc.)
     */
    static int get_rank_class(int hand_rank);

    /**
     * @brief Converts a hand rank to a string description.
     * 
     * @param hand_rank Hand rank to convert
     * @return String description of the hand
     */
    static std::string rank_to_string(int hand_rank);

    /**
     * @brief Converts a hand rank to a percentage value.
     * 
     * @param hand_rank Hand rank to convert
     * @return Percentage value (0.0 to 1.0, higher is better)
     */
    static float get_five_card_rank_percentage(int hand_rank);
}; 