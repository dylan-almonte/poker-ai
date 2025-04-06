#include "card.hpp"
#include <bitset>
#include <sstream>
#include <numeric>

const std::string Card::STR_RANKS = "23456789TJQKA";
const std::array<int, 13> Card::PRIMES = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};

const std::unordered_map<char, int> Card::CHAR_RANK_TO_INT_RANK = {
    {'2', 0}, {'3', 1}, {'4', 2}, {'5', 3}, {'6', 4}, {'7', 5}, {'8', 6},
    {'9', 7}, {'T', 8}, {'J', 9}, {'Q', 10}, {'K', 11}, {'A', 12}
};

const std::unordered_map<char, int> Card::CHAR_SUIT_TO_INT_SUIT = {
    {'s', 1}, // spades
    {'h', 2}, // hearts
    {'d', 4}, // diamonds
    {'c', 8}  // clubs
};

const std::string Card::INT_SUIT_TO_CHAR_SUIT = "xshxdxxxc";

const std::unordered_map<int, std::string> Card::PRETTY_SUITS = {
    {1, "♠"}, // spades
    {2, "♥"}, // hearts
    {4, "♦"}, // diamonds
    {8, "♣"}  // clubs
};

Card::Card(const std::string& card_string) : card_int_(fromString(card_string).toInt()) {}

Card::Card(int card_int) : card_int_(card_int) {}

Card Card::fromString(const std::string& card_string) {
    char rank_char = card_string[0];
    char suit_char = card_string[1];
    
    int rank_int = CHAR_RANK_TO_INT_RANK.at(rank_char);
    int suit_int = CHAR_SUIT_TO_INT_SUIT.at(suit_char);
    int rank_prime = PRIMES[rank_int];

    int bitrank = 1 << rank_int << 16;
    int suit = suit_int << 12;
    int rank = rank_int << 8;

    return Card(bitrank | suit | rank | rank_prime);
}

Card Card::fromInt(int card_int) {
    return Card(card_int);
}

int Card::getRank() const {
    return (card_int_ >> 8) & 0xF;
}

int Card::getSuit() const {
    return (card_int_ >> 12) & 0xF;
}

int Card::getBitRank() const {
    return (card_int_ >> 16) & 0x1FFF;
}

int Card::getPrime() const {
    return card_int_ & 0x3F;
}

std::string Card::toString() const {
    std::string result;
    result += STR_RANKS[getRank()];
    result += INT_SUIT_TO_CHAR_SUIT[getSuit()];
    return result;
}

std::string Card::prettyString() const {
    return "[ " + std::string(1, STR_RANKS[getRank()]) + " " + 
           PRETTY_SUITS.at(getSuit()) + " ]";
}

std::string Card::binaryString() const {
    std::bitset<32> bits(card_int_);
    std::string bstr = bits.to_string();
    std::string result;
    
    for (size_t i = 0; i < bstr.length(); i += 4) {
        if (i > 0) result += " ";
        result += bstr.substr(i, 4);
    }
    
    return result;
}

std::vector<Card> cardsFromStrings(const std::vector<std::string>& card_strs) {
    std::vector<Card> cards;
    for (const auto& card_str : card_strs) {
        cards.push_back(Card(card_str));
    }
    return cards;
}

int primeProductFromHand(const std::vector<Card>& cards) {
    return std::accumulate(cards.begin(), cards.end(), 1,
        [](int product, const Card& card) { return product * card.getPrime(); });
}

int primeProductFromRankbits(int rankbits) {
    int product = 1;
    for (size_t i = 0; i < 13; i++) {
        if (rankbits & (1 << i)) {
            product *= Card::PRIMES[i];
        }
    }
    return product;
}

std::string prettyPrintCards(const std::vector<Card>& cards) {
    std::stringstream ss;
    for (size_t i = 0; i < cards.size(); i++) {
        if (i > 0) ss << " ";
        ss << cards[i].prettyString();
    }
    return ss.str();
} 