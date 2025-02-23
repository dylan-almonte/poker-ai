#pragma once

#include <vector>
#include <memory>
#include "card.hpp"
#include "lookup_table.hpp"

class Evaluator {
public:
    Evaluator();

    // Main evaluation methods
    int evaluate(const std::vector<std::shared_ptr<Card>>& cards,
                const std::vector<std::shared_ptr<Card>>& board);
    
    // Hand ranking methods
    int getRankClass(int hand_rank);
    std::string classToString(int class_int);
    double getFiveCardRankPercentage(int hand_rank);

    // Hand evaluation helpers
    int evaluateFiveCards(const std::vector<int>& cards);
    int evaluateSixCards(const std::vector<int>& cards);
    int evaluateSevenCards(const std::vector<int>& cards);

private:
    std::unique_ptr<LookupTable> table_;

    static int calculatePrimeProduct(const std::vector<int>& cards);
    static int calculatePrimeProduct(int rankbits);
    
    static constexpr int PRIMES[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
}; 