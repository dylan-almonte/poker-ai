#include "evaluation/eval_card.hpp"
#include <sstream>
#include <iostream>
#include <bitset>
#include <algorithm>

// Initialize static constants
const std::string EvaluationCard::STR_RANKS = "23456789TJQKA";
const std::array<int, 13> EvaluationCard::PRIMES = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};

// Initialize rank mapping
const std::map<char, int> EvaluationCard::CHAR_RANK_TO_INT_RANK = {
    {'2', 0}, {'3', 1}, {'4', 2}, {'5', 3}, {'6', 4}, {'7', 5}, {'8', 6},
    {'9', 7}, {'T', 8}, {'J', 9}, {'Q', 10}, {'K', 11}, {'A', 12}
};

// Initialize suit mapping
const std::map<char, int> EvaluationCard::CHAR_SUIT_TO_INT_SUIT = {
    {'s', 1},  // spades
    {'h', 2},  // hearts
    {'d', 4},  // diamonds
    {'c', 8}   // clubs
};

const std::string EvaluationCard::INT_SUIT_TO_CHAR_SUIT = "xshxdxxxc";

const std::map<int, std::string> EvaluationCard::PRETTY_SUITS = {
    {1, "♠"},  // spades
    {2, "♥"},  // hearts
    {4, "♦"},  // diamonds
    {8, "♣"}   // clubs
};

const std::array<int, 2> EvaluationCard::PRETTY_REDS = {2, 4};

int EvaluationCard::New(const std::string& card_str) {
    char rank_char = card_str[0];
    char suit_char = card_str[1];
    
    int rank_int = CHAR_RANK_TO_INT_RANK.at(rank_char);
    int suit_int = CHAR_SUIT_TO_INT_SUIT.at(suit_char);
    int rank_prime = PRIMES[rank_int];

    int bitrank = (1 << rank_int) << 16;
    int suit = suit_int << 12;
    int rank = rank_int << 8;

    return bitrank | suit | rank | rank_prime;
}

std::string EvaluationCard::IntToStr(int card_int) {
    int rank_int = GetRankInt(card_int);
    int suit_int = GetSuitInt(card_int);
    
    std::string result;
    result += STR_RANKS[rank_int];
    result += INT_SUIT_TO_CHAR_SUIT[suit_int];
    return result;
}

int EvaluationCard::GetRankInt(int card_int) {
    return (card_int >> 8) & 0xF;
}

int EvaluationCard::GetSuitInt(int card_int) {
    return (card_int >> 12) & 0xF;
}

int EvaluationCard::GetBitrankInt(int card_int) {
    return (card_int >> 16) & 0x1FFF;
}

int EvaluationCard::GetPrime(int card_int) {
    return card_int & 0x3F;
}

std::vector<int> EvaluationCard::HandToBinary(const std::vector<std::string>& card_strs) {
    std::vector<int> binary_hand;
    binary_hand.reserve(card_strs.size());
    
    for (const auto& card : card_strs) {
        binary_hand.push_back(New(card));
    }
    return binary_hand;
}

int EvaluationCard::PrimeProductFromHand(const std::vector<int>& card_ints) {
    int product = 1;
    for (int card : card_ints) {
        product *= (card & 0xFF);
    }
    return product;
}

int EvaluationCard::PrimeProductFromRankbits(int rankbits) {
    int product = 1;
    for (int i = 0; i < 13; i++) {
        if (rankbits & (1 << i)) {
            product *= PRIMES[i];
        }
    }
    return product;
}

std::string EvaluationCard::IntToBinary(int card_int) {
    std::bitset<32> b(card_int);
    std::string binary = b.to_string();
    
    // Format with spaces every 4 bits
    std::string formatted;
    for (size_t i = 0; i < binary.length(); i++) {
        if (i > 0 && i % 4 == 0) {
            formatted += " ";
        }
        formatted += binary[i];
    }
    return formatted;
}

std::string EvaluationCard::IntToPrettyStr(int card_int) {
    int suit_int = GetSuitInt(card_int);
    int rank_int = GetRankInt(card_int);
    
    std::string suit_str = PRETTY_SUITS.at(suit_int);
    char rank_char = STR_RANKS[rank_int];
    
    return std::string("[") + rank_char + suit_str + "]";
}

void EvaluationCard::PrintPrettyCard(int card_int) {
    std::cout << IntToPrettyStr(card_int) << std::endl;
}

void EvaluationCard::PrintPrettyCards(const std::vector<int>& card_ints) {
    std::string output = " ";
    for (size_t i = 0; i < card_ints.size(); i++) {
        output += IntToPrettyStr(card_ints[i]);
        if (i != card_ints.size() - 1) {
            output += ",";
        }
    }
    output += " ";
    std::cout << output << std::endl;
} 