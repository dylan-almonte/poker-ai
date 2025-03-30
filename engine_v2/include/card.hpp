#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

class Card {
private:
    int card_int_;
    static const std::string INT_SUIT_TO_CHAR_SUIT;
    static const std::unordered_map<int, std::string> PRETTY_SUITS;

public:
    static const std::unordered_map<char, int> CHAR_SUIT_TO_INT_SUIT;
    static const std::unordered_map<char, int> CHAR_RANK_TO_INT_RANK;
    static const std::string STR_RANKS;
    static const std::array<int, 13> PRIMES;

    // Constructors
    Card(const std::string& card_string);
    Card(int card_int);

    // Static factory methods
    static Card fromString(const std::string& card_string);
    static Card fromInt(int card_int);

    // Getters
    int getRank() const;
    int getSuit() const;
    int getBitRank() const;
    int getPrime() const;
    int toInt() const { return card_int_; }

    // String representations
    std::string toString() const;
    std::string prettyString() const;
    std::string binaryString() const;

    // Operator overloads
    bool operator==(const Card& other) const { return card_int_ == other.card_int_; }
    bool operator!=(const Card& other) const { return !(*this == other); }
};

// Helper functions
std::vector<Card> cardsFromStrings(const std::vector<std::string>& card_strs);
int primeProductFromHand(const std::vector<Card>& cards);
int primeProductFromRankbits(int rankbits);
std::string prettyPrintCards(const std::vector<Card>& cards); 