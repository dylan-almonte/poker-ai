#pragma once
#include "card.hpp"
#include <vector>
#include <unordered_map>

class LookupTable {
public:
    static const int MAX_STRAIGHT_FLUSH = 10;
    static const int MAX_FOUR_OF_A_KIND = 166;
    static const int MAX_FULL_HOUSE = 322;
    static const int MAX_FLUSH = 1599;
    static const int MAX_STRAIGHT = 1609;
    static const int MAX_THREE_OF_A_KIND = 2467;
    static const int MAX_TWO_PAIR = 3325;
    static const int MAX_PAIR = 6185;
    static const int MAX_HIGH_CARD = 7462;

    static const std::unordered_map<int, int> MAX_TO_RANK_CLASS;
    static const std::unordered_map<int, std::string> RANK_CLASS_TO_STRING;

    std::unordered_map<int, int> flush_lookup_;
    std::unordered_map<int, int> unsuited_lookup_  ;

    LookupTable();

private:
    void _flushes();
    void _multiples();
    void _straight_and_highcards(const std::vector<int>& straights, const std::vector<int>& highcards);
    static std::vector<int> _get_lexographically_next_bit_sequence(int bits);
};

class Evaluator {
private:
    static LookupTable lookup_table;

public:
    static int evaluate(const std::vector<Card>& cards, const std::vector<Card>& board);
    static int _five(const std::vector<Card>& cards);
    static int get_rank_class(int hand_rank);
    static std::string rank_to_string(int hand_rank);
    static float get_five_card_rank_percentage(int hand_rank);
}; 