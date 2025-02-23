#pragma once

#include <string>
#include <set>
#include <vector>

class Card {
public:
    Card(int rank, const std::string& suit);
    Card(const std::string& rank, const std::string& suit);

    // Getters
    int getRankInt() const { return rank_; }
    std::string getRankStr() const;
    const std::string& getSuit() const { return suit_; }
    int getEvalCard() const { return eval_card_; }

    // Comparison operators
    bool operator<(const Card& other) const { return rank_ < other.rank_; }
    bool operator<=(const Card& other) const { return rank_ <= other.rank_; }
    bool operator>(const Card& other) const { return rank_ > other.rank_; }
    bool operator>=(const Card& other) const { return rank_ >= other.rank_; }
    bool operator==(const Card& other) const { return eval_card_ == other.eval_card_; }
    bool operator!=(const Card& other) const { return eval_card_ != other.eval_card_; }

    // Static methods
    static std::set<std::string> getAllSuits();
    static std::vector<std::string> getAllRanks();

private:
    int strToRank(const std::string& str);
    std::string rankToStr(int rank) const;
    char rankToChar(int rank) const;
    std::string suitToIcon(const std::string& suit) const;

    int rank_;
    std::string suit_;
    int eval_card_;
}; 