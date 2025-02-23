#include "evaluator.hpp"
#include "lookup_table.hpp"
#include <algorithm>
#include <numeric>

Evaluator::Evaluator() 
    : table_(std::make_unique<LookupTable>()) {
}

int Evaluator::evaluate(const std::vector<std::shared_ptr<Card>>& cards,
                       const std::vector<std::shared_ptr<Card>>& board) {
    std::vector<int> eval_cards;
    
    // Convert cards to evaluation format
    for (const auto& card : cards) {
        eval_cards.push_back(card->getEvalCard());
    }
    for (const auto& card : board) {
        eval_cards.push_back(card->getEvalCard());
    }

    // Evaluate based on number of cards
    switch (eval_cards.size()) {
        case 5: return evaluateFiveCards(eval_cards);
        case 6: return evaluateSixCards(eval_cards);
        case 7: return evaluateSevenCards(eval_cards);
        default:
            throw std::invalid_argument("Invalid number of cards for evaluation");
    }
}

int Evaluator::evaluateFiveCards(const std::vector<int>& cards) {
    // Check if flush
    int first_suit = cards[0] & 0xF000;
    bool is_flush = std::all_of(cards.begin(), cards.end(),
        [first_suit](int card) { return (card & 0xF000) == first_suit; });

    if (is_flush) {
        // Calculate hand OR and prime product for flush lookup
        int hand_OR = std::accumulate(cards.begin(), cards.end(), 0, 
            [](int a, int b) { return a | (b >> 16); });
        int prime_product = calculatePrimeProduct(hand_OR);
        return table_->getFlushRank(prime_product);
    } else {
        // Calculate prime product for unsuited lookup
        int prime_product = calculatePrimeProduct(cards);
        return table_->getUnsuitedRank(prime_product);
    }
}

int Evaluator::evaluateSixCards(const std::vector<int>& cards) {
    int min_rank = LookupTable::MAX_HIGH_CARD;
    
    // Evaluate all 5-card combinations
    for (int i = 0; i < 6; ++i) {
        std::vector<int> five_cards;
        for (int j = 0; j < 6; ++j) {
            if (j != i) {
                five_cards.push_back(cards[j]);
            }
        }
        min_rank = std::min(min_rank, evaluateFiveCards(five_cards));
    }
    
    return min_rank;
}

int Evaluator::evaluateSevenCards(const std::vector<int>& cards) {
    int min_rank = LookupTable::MAX_HIGH_CARD;
    
    // Evaluate all 5-card combinations
    for (int i = 0; i < 21; ++i) {
        std::vector<int> five_cards;
        int skip1 = i / 6;
        int skip2 = i % 6;
        if (skip2 >= skip1) skip2++;
        
        for (int j = 0; j < 7; ++j) {
            if (j != skip1 && j != skip2) {
                five_cards.push_back(cards[j]);
            }
        }
        min_rank = std::min(min_rank, evaluateFiveCards(five_cards));
    }
    
    return min_rank;
}

int Evaluator::getRankClass(int hand_rank) {
    return LookupTable::maxToRankClass(hand_rank);
}

std::string Evaluator::classToString(int class_int) {
    return LookupTable::rankClassToString(class_int);
}

double Evaluator::getFiveCardRankPercentage(int hand_rank) {
    return static_cast<double>(hand_rank) / LookupTable::MAX_HIGH_CARD;
}

int Evaluator::calculatePrimeProduct(const std::vector<int>& cards) {
    int product = 1;
    for (int card : cards) {
        int rank = (card >> 8) & 0xF;
        product *= PRIMES[rank];
    }
    return product;
}

int Evaluator::calculatePrimeProduct(int rankbits) {
    int product = 1;
    for (int i = 0; i < 13; i++) {
        if (rankbits & (1 << i)) {
            product *= PRIMES[i];
        }
    }
    return product;
} 