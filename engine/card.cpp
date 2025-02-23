#include "card.hpp"
#include <stdexcept>
#include <unordered_map>

Card::Card(int rank, const std::string& suit) {
    if (rank < 2 || rank > 14) {
        throw std::invalid_argument("Rank must be between 2 and 14");
    }
    if (getAllSuits().find(suit) == getAllSuits().end()) {
        throw std::invalid_argument("Invalid suit");
    }
    rank_ = rank;
    suit_ = suit;
    
    // TODO: Implement eval_card_ calculation similar to EvaluationCard
}

Card::Card(const std::string& rank, const std::string& suit) 
    : Card(strToRank(rank), suit) {
}

std::set<std::string> Card::getAllSuits() {
    return {"spades", "diamonds", "clubs", "hearts"};
}

std::vector<std::string> Card::getAllRanks() {
    return {"2", "3", "4", "5", "6", "7", "8", "9", "10", 
            "jack", "queen", "king", "ace"};
}

int Card::strToRank(const std::string& str) {
    static const std::unordered_map<std::string, int> rankMap = {
        {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6},
        {"7", 7}, {"8", 8}, {"9", 9}, {"10", 10}, {"t", 10},
        {"jack", 11}, {"j", 11}, {"queen", 12}, {"q", 12},
        {"king", 13}, {"k", 13}, {"ace", 14}, {"a", 14}
    };

    auto it = rankMap.find(str);
    if (it == rankMap.end()) {
        throw std::invalid_argument("Invalid rank string");
    }
    return it->second;
}

std::string Card::rankToStr(int rank) const {
    static const std::unordered_map<int, std::string> rankMap = {
        {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}, {6, "6"},
        {7, "7"}, {8, "8"}, {9, "9"}, {10, "10"},
        {11, "jack"}, {12, "queen"}, {13, "king"}, {14, "ace"}
    };

    auto it = rankMap.find(rank);
    if (it == rankMap.end()) {
        throw std::invalid_argument("Invalid rank");
    }
    return it->second;
}

std::string Card::suitToIcon(const std::string& suit) const {
    static const std::unordered_map<std::string, std::string> iconMap = {
        {"hearts", "♥"}, {"diamonds", "♦"}, 
        {"clubs", "♣"}, {"spades", "♠"}
    };

    auto it = iconMap.find(suit);
    if (it == iconMap.end()) {
        throw std::invalid_argument("Invalid suit");
    }
    return it->second;
} 