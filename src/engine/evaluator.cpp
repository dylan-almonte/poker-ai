#include "include/engine/evaluator.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>

// Initialize static members
LookupTable Evaluator::lookup_table;

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

LookupTable::LookupTable() {
    _flushes();
    _multiples();
}

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

int Evaluator::evaluate(const std::vector<Card>& cards, const std::vector<Card>& board) {
    std::vector<Card> all_cards;
    all_cards.insert(all_cards.end(), cards.begin(), cards.end());
    all_cards.insert(all_cards.end(), board.begin(), board.end());

    int min_rank = LookupTable::MAX_HIGH_CARD;
    std::vector<std::vector<Card>> combinations;
    
    // TODO: change to use n choose 5
    // Generate all 5-card combinations
    for (size_t i = 0; i < all_cards.size() - 4; i++) {
        for (size_t j = i + 1; j < all_cards.size() - 3; j++) {
            for (size_t k = j + 1; k < all_cards.size() - 2; k++) {
                for (size_t l = k + 1; l < all_cards.size() - 1; l++) {
                    for (size_t m = l + 1; m < all_cards.size(); m++) {
                        std::vector<Card> hand = {all_cards[i], all_cards[j], all_cards[k], 
                                                all_cards[l], all_cards[m]};
                        min_rank = std::min(min_rank, _five(hand));
                    }
                }
            }
        }
    }
    
    return min_rank;
}

int Evaluator::_five(const std::vector<Card>& cards) {
    if (cards.size() != 5) {
        throw std::invalid_argument("_five function requires exactly 5 cards");
    }

    // Check for flush
    int flush_check = cards[0].toInt() & cards[1].toInt() & cards[2].toInt() & 
                     cards[3].toInt() & cards[4].toInt() & 0xF000;

    if (flush_check) {
        int hand_or = (cards[0].toInt() | cards[1].toInt() | cards[2].toInt() | 
                      cards[3].toInt() | cards[4].toInt()) >> 16;
        int prime = primeProductFromRankbits(hand_or);
        return lookup_table.flush_lookup_[prime];
    }

    // Otherwise
    int prime = primeProductFromHand(cards);
    return lookup_table.unsuited_lookup_[prime];
}

int Evaluator::get_rank_class(int hand_rank) {
    int max_rank = LookupTable::MAX_HIGH_CARD;
    for (const auto& [rank, _] : LookupTable::MAX_TO_RANK_CLASS) {
        if (hand_rank <= rank) {
            max_rank = std::min(max_rank, rank);
        }
    }
    return LookupTable::MAX_TO_RANK_CLASS.at(max_rank);
}

std::string Evaluator::rank_to_string(int hand_rank) {
    return LookupTable::RANK_CLASS_TO_STRING.at(get_rank_class(hand_rank));
}

float Evaluator::get_five_card_rank_percentage(int hand_rank) {
    return 1.0f - static_cast<float>(hand_rank) / static_cast<float>(LookupTable::MAX_HIGH_CARD);
} 