#pragma once
#include <string>
#include <vector>
#include <map>
#include <array>

class EvaluationCard {
public:
    // Move PRIMES from private to public
    static const std::array<int, 13> PRIMES;

    // Static methods
    static int New(const std::string& card_str);
    static std::string IntToStr(int card_int);
    static int GetRankInt(int card_int);
    static int GetSuitInt(int card_int);
    static int GetBitrankInt(int card_int);
    static int GetPrime(int card_int);
    static std::vector<int> HandToBinary(const std::vector<std::string>& card_strs);
    static int PrimeProductFromHand(const std::vector<int>& card_ints);
    static int PrimeProductFromRankbits(int rankbits);
    static std::string IntToBinary(int card_int);
    static std::string IntToPrettyStr(int card_int);
    static void PrintPrettyCard(int card_int);
    static void PrintPrettyCards(const std::vector<int>& card_ints);

private:
    static const std::string STR_RANKS;
    static const std::map<char, int> CHAR_RANK_TO_INT_RANK;
    static const std::map<char, int> CHAR_SUIT_TO_INT_SUIT;
    static const std::string INT_SUIT_TO_CHAR_SUIT;
    static const std::map<int, std::string> PRETTY_SUITS;
    static const std::array<int, 2> PRETTY_REDS;
}; 