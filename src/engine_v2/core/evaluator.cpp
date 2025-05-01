#include "include/engine/evaluator.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>

// Initialize static members
std::array<int, 0x1FFF> Evaluator::flush_lookup_;
std::array<int, 0x1FFF> Evaluator::unsuited_lookup_;

/**
 * @brief Maps maximum ranks to their corresponding rank classes.
 * 
 * Used to determine the type of hand from its rank.
 */
const std::unordered_map<int, int> LookupTable::MAX_TO_RANK_CLASS = {
    {MAX_STRAIGHT_FLUSH, 1},
    {MAX_FOUR_OF_A_KIND, 2},
    {MAX_FULL_HOUSE, 3},
    {MAX_FLUSH, 4},
    {MAX_STRAIGHT, 5},
    {MAX_THREE_OF_A_KIND, 6},
    {MAX_TWO_PAIR, 7},
    {MAX_PAIR, 8},
    {MAX_HIGH_CARD, 9}
};

/**
 * @brief Maps rank classes to their string descriptions.
 * 
 * Used to convert hand ranks to human-readable strings.
 */
const std::unordered_map<int, std::string> LookupTable::RANK_CLASS_TO_STRING = {
    {1, "Straight Flush"},
    {2, "Four of a Kind"},
    {3, "Full House"},
    {4, "Flush"},
    {5, "Straight"},
    {6, "Three of a Kind"},
    {7, "Two Pair"},
    {8, "Pair"},
    {9, "High Card"}
};

/**
 * @brief Constructs a new lookup table.
 * 
 * Initializes the lookup tables for all hand types.
 */
LookupTable::LookupTable() {
    _flushes();
    _multiples();
}

/**
 * @brief Initializes the flush lookup table.
 * 
 * Generates and stores all possible flush hands in order of strength.
 */
void LookupTable::_flushes() {
    // Straight flushes in rank order
    std::vector<int> straight_flushes = {
        7936,  // int('0b1111100000000', 2), # royal flush
        3968,  // int('0b111110000000', 2),
        1984,  // int('0b11111000000', 2),
        992,   // int('0b1111100000', 2),
        496,   // int('0b111110000', 2),
        248,   // int('0b11111000', 2),
        124,   // int('0b1111100', 2),
        62,    // int('0b111110', 2),
        31,    // int('0b11111', 2),
        4111   // int('0b1000000001111', 2) # 5 high
    };

    // Generate all other flushes
    std::vector<int> flushes;
    auto gen = _get_lexographically_next_bit_sequence(0b11111);

    for (size_t i = 0; i < 1277 + straight_flushes.size() - 1; i++) {
        int flush = gen[i];
        
        bool not_sf = true;
        for (int straight_flush : straight_flushes) {
            if ((flush ^ straight_flush) == 0) {
                not_sf = false;
                break;
            }
        }

        if (not_sf) {
            flushes.push_back(flush);
        }
    }

    std::reverse(flushes.begin(), flushes.end());

    // Add to lookup tables
    int rank = 1;
    for (int straight_flush : straight_flushes) {
        int prime_product = primeProductFromRankbits(straight_flush);
        flush_lookup_[prime_product] = rank++;
    }

    rank = MAX_FULL_HOUSE + 1;
    for (int flush : flushes) {
        int prime_product = primeProductFromRankbits(flush);
        flush_lookup_[prime_product] = rank++;
    }

    _straight_and_highcards(straight_flushes, flushes);
}

/**
 * @brief Initializes the lookup tables for straights and high cards.
 * 
 * @param straights Vector of straight bit patterns
 * @param highcards Vector of high card bit patterns
 */
void LookupTable::_straight_and_highcards(const std::vector<int>& straights, const std::vector<int>& highcards) {
    int rank = MAX_FLUSH + 1;
    for (int straight : straights) {
        int prime_product = primeProductFromRankbits(straight);
        unsuited_lookup_[prime_product] = rank++;
    }

    rank = MAX_PAIR + 1;
    for (int high : highcards) {
        int prime_product = primeProductFromRankbits(high);
        unsuited_lookup_[prime_product] = rank++;
    }
}

/**
 * @brief Initializes the lookup table for multiple cards of same rank.
 * 
 * Generates and stores all possible hands with multiple cards of the same rank
 * (four of a kind, full house, three of a kind, two pair, pair).
 */
void LookupTable::_multiples() {
    // Four of a kind
    int rank = MAX_STRAIGHT_FLUSH + 1;
    for (int i = 12; i >= 0; i--) {
        int kicker_ranks = (1 << 13) - 1;
        kicker_ranks &= ~(1 << i);
        
        for (int j = 12; j >= 0; j--) {
            if ((kicker_ranks >> j) & 1) {
                int product = std::pow(Card::PRIMES[i], 4) * Card::PRIMES[j];
                unsuited_lookup_[product] = rank++;
            }
        }
    }

    // Full house
    rank = MAX_FOUR_OF_A_KIND + 1;
    for (int i = 12; i >= 0; i--) {
        int pair_ranks = (1 << 13) - 1;
        pair_ranks &= ~(1 << i);
        
        for (int j = 12; j >= 0; j--) {
            if ((pair_ranks >> j) & 1) {
                int product = std::pow(Card::PRIMES[i], 3) * std::pow(Card::PRIMES[j], 2);
                unsuited_lookup_[product] = rank++;
            }
        }
    }

    // Three of a kind
    rank = MAX_STRAIGHT + 1;
    for (int i = 12; i >= 0; i--) {
        int kicker_ranks = (1 << 13) - 1;
        kicker_ranks &= ~(1 << i);
        
        for (int j = 12; j >= 0; j--) {
            if ((kicker_ranks >> j) & 1) {
                for (int k = j - 1; k >= 0; k--) {
                    if ((kicker_ranks >> k) & 1) {
                        int product = std::pow(Card::PRIMES[i], 3) * Card::PRIMES[j] * Card::PRIMES[k];
                        unsuited_lookup_[product] = rank++;
                    }
                }
            }
        }
    }

    // Two pair
    rank = MAX_THREE_OF_A_KIND + 1;
    for (int i = 12; i >= 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            int kicker_ranks = (1 << 13) - 1;
            kicker_ranks &= ~(1 << i);
            kicker_ranks &= ~(1 << j);
            
            for (int k = 12; k >= 0; k--) {
                if ((kicker_ranks >> k) & 1) {
                    int product = std::pow(Card::PRIMES[i], 2) * std::pow(Card::PRIMES[j], 2) * Card::PRIMES[k];
                    unsuited_lookup_[product] = rank++;
                }
            }
        }
    }

    // Pair
    rank = MAX_TWO_PAIR + 1;
    for (int i = 12; i >= 0; i--) {
        int kicker_ranks = (1 << 13) - 1;
        kicker_ranks &= ~(1 << i);
        
        for (int j = 12; j >= 0; j--) {
            if ((kicker_ranks >> j) & 1) {
                for (int k = j - 1; k >= 0; k--) {
                    if ((kicker_ranks >> k) & 1) {
                        for (int l = k - 1; l >= 0; l--) {
                            if ((kicker_ranks >> l) & 1) {
                                int product = std::pow(Card::PRIMES[i], 2) * Card::PRIMES[j] * Card::PRIMES[k] * Card::PRIMES[l];
                                unsuited_lookup_[product] = rank++;
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Generates lexicographically next bit sequence.
 * 
 * @param bits Starting bit sequence
 * @return Vector of next bit sequences
 */
std::vector<int> LookupTable::_get_lexographically_next_bit_sequence(int bits) {
    std::vector<int> sequences;
    int current = bits;
    
    while (current < (1 << 13)) {
        int t = (current | (current - 1)) + 1;
        current = t | ((((t & -t) / (current & -current)) >> 1) - 1);
        sequences.push_back(current);
    }
    
    return sequences;
}

/**
 * @brief Fast 7-card evaluation using bitwise operations.
 * 
 * @param cards Vector of 7 cards to evaluate
 * @return Hand rank (lower is better)
 */
int Evaluator::evaluate7(const std::vector<Card>& cards) {
    int flush_suit = 0;
    int suit_hist = 0;
    int rank_hist = 0;
    
    // Single pass over cards to build histograms
    for (const auto& card : cards) {
        int suit = card.getSuit();
        suit_hist |= (1 << suit);
        if ((suit_hist >> suit) & 1) {
            flush_suit = suit;
        }
        rank_hist |= (1 << card.getRank());
    }
    
    // Check for flush first (most decisive)
    if (flush_suit) {
        return flush_lookup_[rank_hist];
    }
    
    // Otherwise evaluate using rank patterns
    return unsuited_lookup_[rank_hist];
}

/**
 * @brief Evaluates a poker hand against a board.
 * 
 * @param cards Player's hole cards
 * @param board Community cards
 * @return Hand rank (lower is better)
 */
int Evaluator::evaluate(const std::vector<Card>& cards, const std::vector<Card>& board) {
    // Combine all cards
    std::vector<Card> all_cards;
    all_cards.reserve(cards.size() + board.size());
    all_cards.insert(all_cards.end(), cards.begin(), cards.end());
    all_cards.insert(all_cards.end(), board.begin(), board.end());

    // If we have exactly 7 cards, use the fast evaluation
    if (all_cards.size() == 7) {
        return evaluate7(all_cards);
    }

    // If we have more than 7 cards, find the best 7-card combination
    int min_rank = LookupTable::MAX_HIGH_CARD;
    
    // Generate all 7-card combinations using an efficient approach
    std::vector<size_t> indices(7);
    for (size_t i = 0; i < 7; ++i) {
        indices[i] = i;
    }

    while (true) {
        // Evaluate current combination
        std::vector<Card> hand;
        hand.reserve(7);
        for (size_t i = 0; i < 7; ++i) {
            hand.push_back(all_cards[indices[i]]);
        }
        
        int rank = evaluate7(hand);
        if (rank < min_rank) {
            min_rank = rank;
            // If we find a straight flush, it's the best possible hand
            if (min_rank <= LookupTable::MAX_STRAIGHT_FLUSH) {
                return min_rank;
            }
        }

        // Generate next combination
        size_t i = 6;
        while (i != static_cast<size_t>(-1) && indices[i] == all_cards.size() - 7 + i) {
            --i;
        }
        
        if (i == static_cast<size_t>(-1)) {
            break;
        }
        
        ++indices[i];
        for (size_t j = i + 1; j < 7; ++j) {
            indices[j] = indices[j - 1] + 1;
        }
    }
    
    return min_rank;
}

/**
 * @brief Gets the rank class of a hand rank.
 * 
 * @param hand_rank Hand rank to classify
 * @return Rank class (1=straight flush, 2=four of a kind, etc.)
 */
int Evaluator::get_rank_class(int hand_rank) {
    int max_rank = LookupTable::MAX_HIGH_CARD;
    for (const auto& [rank, _] : LookupTable::MAX_TO_RANK_CLASS) {
        if (hand_rank <= rank) {
            max_rank = std::min(max_rank, rank);
        }
    }
    return LookupTable::MAX_TO_RANK_CLASS.at(max_rank);
}

/**
 * @brief Converts a hand rank to a string description.
 * 
 * @param hand_rank Hand rank to convert
 * @return String description of the hand
 */
std::string Evaluator::rank_to_string(int hand_rank) {
    return LookupTable::RANK_CLASS_TO_STRING.at(get_rank_class(hand_rank));
}

/**
 * @brief Converts a hand rank to a percentage value.
 * 
 * @param hand_rank Hand rank to convert
 * @return Percentage value (0.0 to 1.0, higher is better)
 */
float Evaluator::get_five_card_rank_percentage(int hand_rank) {
    return 1.0f - static_cast<float>(hand_rank) / static_cast<float>(LookupTable::MAX_HIGH_CARD);
} 