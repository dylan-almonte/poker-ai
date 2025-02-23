#include "lookup_table.hpp"
#include <algorithm>
#include <numeric>

// Static member initialization
const std::unordered_map<int, int> LookupTable::MAX_TO_RANK_CLASS_MAP = {
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

const std::unordered_map<int, std::string> LookupTable::RANK_CLASS_TO_STRING_MAP = {
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
    initFlushLookup();
    initUnsuitedLookup();
    initRankClasses();
}

void LookupTable::initFlushLookup() {
    // TODO: Implement flush lookup table initialization
}

void LookupTable::initUnsuitedLookup() {
    // TODO: Implement unsuited lookup table initialization
}

void LookupTable::initRankClasses() {
    // TODO: Implement rank class lookup initialization
}

int LookupTable::getFlushRank(int prime_product) const {
    auto it = flush_lookup_.find(prime_product);
    return it != flush_lookup_.end() ? it->second : -1;
}

int LookupTable::getUnsuitedRank(int prime_product) const {
    auto it = unsuited_lookup_.find(prime_product);
    return it != unsuited_lookup_.end() ? it->second : -1;
}

int LookupTable::getRankClass(int hand_rank) const {
    auto it = rank_class_lookup_.find(hand_rank);
    return it != rank_class_lookup_.end() ? it->second : -1;
}

std::string LookupTable::classToString(int class_int) const {
    auto it = class_strings_.find(class_int);
    return it != class_strings_.end() ? it->second : "Invalid Class";
}

double LookupTable::getFiveCardRankPercentage(int hand_rank) const {
    // TODO: Implement percentage calculation
    return 0.0;
}

void LookupTable::generateLookupTables() {
    generateFlushes();
    generateMultiples();
}

void LookupTable::generateFlushes() {
    // Straight flushes in rank order
    std::vector<int> straight_flushes = {
        0b1111100000000,  // royal flush
        0b0111110000000,
        0b0011111000000,
        0b0001111100000,
        0b0000111110000,
        0b0000011111000,
        0b0000001111100,
        0b0000000111110,
        0b0000000011111,
        0b1000000001111   // 5 high
    };

    // Generate all other flushes
    std::vector<int> flushes;
    auto gen = getLexographicallyNextBitSequence(0b11111);

    for (int i = 0; i < 1277 + straight_flushes.size() - 1; ++i) {
        int next = gen[i];
        bool not_sf = true;
        
        // Check if this pattern matches a straight flush
        for (int sf : straight_flushes) {
            if ((next ^ sf) == 0) {
                not_sf = false;
                break;
            }
        }

        if (not_sf) {
            flushes.push_back(next);
        }
    }

    // Reverse for correct ranking order
    std::reverse(flushes.begin(), flushes.end());

    // Fill lookup tables
    fillLookupTable(1, straight_flushes, flush_lookup_);
    fillLookupTable(MAX_FULL_HOUSE + 1, flushes, flush_lookup_);
}

void LookupTable::generateMultiples() {
    // Generate rankings for four of a kind, full house, etc.
    std::vector<int> ranks(13);
    std::iota(ranks.begin(), ranks.end(), 0);
    std::reverse(ranks.begin(), ranks.end());

    // Four of a kind
    int rank = MAX_STRAIGHT_FLUSH + 1;
    for (int i : ranks) {
        std::vector<int> kickers = ranks;
        kickers.erase(std::find(kickers.begin(), kickers.end(), i));
        for (int k : kickers) {
            int product = static_cast<int>(std::pow(PRIMES[i], 4) * PRIMES[k]);
            unsuited_lookup_[product] = rank++;
        }
    }

    // Full house
    rank = MAX_FOUR_OF_A_KIND + 1;
    for (int i : ranks) {
        std::vector<int> pair_ranks = ranks;
        pair_ranks.erase(std::find(pair_ranks.begin(), pair_ranks.end(), i));
        for (int pr : pair_ranks) {
            int product = static_cast<int>(std::pow(PRIMES[i], 3) * std::pow(PRIMES[pr], 2));
            unsuited_lookup_[product] = rank++;
        }
    }

    // Continue with three of a kind, two pair, and pair...
    // (Implementation continues with similar pattern)
}

void LookupTable::fillLookupTable(int rank_init, const std::vector<int>& rankbits,
                                 std::unordered_map<int, int>& lookup_table) {
    int rank = rank_init;
    for (int rb : rankbits) {
        int prime_product = calculatePrimeProduct(rb);
        lookup_table[prime_product] = rank++;
    }
}

std::vector<int> LookupTable::getLexographicallyNextBitSequence(int bits) {
    std::vector<int> sequences;
    int current = bits;
    
    while (true) {
        sequences.push_back(current);
        int t = (current | (current - 1)) + 1;
        int next = t | ((((t & -t) / (current & -current)) >> 1) - 1);
        if (next > 0b11111111111111) break;  // 13 bits
        current = next;
    }
    
    return sequences;
}

int LookupTable::maxToRankClass(int max_rank) {
    for (const auto& [max_val, rank_class] : MAX_TO_RANK_CLASS_MAP) {
        if (max_rank <= max_val) {
            return rank_class;
        }
    }
    throw std::invalid_argument("Invalid hand rank");
}

std::string LookupTable::rankClassToString(int rank_class) {
    auto it = RANK_CLASS_TO_STRING_MAP.find(rank_class);
    if (it == RANK_CLASS_TO_STRING_MAP.end()) {
        throw std::invalid_argument("Invalid rank class");
    }
    return it->second;
} 