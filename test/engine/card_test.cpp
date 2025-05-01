#include <gtest/gtest.h>
#include "../../src/include/engine_v2/card.hpp"

TEST(CardTest, ConstructorFromString) {
    Card card("As");
    EXPECT_EQ(card.getRank(), 12);  // Ace
    EXPECT_EQ(card.getSuit(), 1);   // Spades

    Card card2("Kh");
    EXPECT_EQ(card2.getRank(), 11); // King
    EXPECT_EQ(card2.getSuit(), 2);  // Hearts

    Card card3("2c");
    EXPECT_EQ(card3.getRank(), 0);  // 2
    EXPECT_EQ(card3.getSuit(), 8);  // Clubs
}

TEST(CardTest, ConstructorFromInt) {
    Card card(0x00010000 | 0x00001000 | 0x00000100 | 0x0000002D); // Ace of Spades
    EXPECT_EQ(card.getRank(), 12);
    EXPECT_EQ(card.getSuit(), 1);
}

TEST(CardTest, FromString) {
    Card card = Card::fromString("As");
    EXPECT_EQ(card.getRank(), 12);
    EXPECT_EQ(card.getSuit(), 1);
}

TEST(CardTest, FromInt) {
    Card card = Card::fromInt(0x00010000 | 0x00001000 | 0x00000100 | 0x0000002D);
    EXPECT_EQ(card.getRank(), 12);
    EXPECT_EQ(card.getSuit(), 1);
}

TEST(CardTest, GetBitRank) {
    Card card("As");
    EXPECT_EQ(card.getBitRank(), 1 << 12);
}

TEST(CardTest, GetPrime) {
    Card card("As");
    EXPECT_EQ(card.getPrime(), 41); // Ace's prime
}

TEST(CardTest, ToString) {
    Card card("As");
    EXPECT_EQ(card.toString(), "As");

    Card card2("Kh");
    EXPECT_EQ(card2.toString(), "Kh");

    Card card3("2c");
    EXPECT_EQ(card3.toString(), "2c");
}

TEST(CardTest, PrettyString) {
    Card card("As");
    EXPECT_EQ(card.prettyString(), "[ A ♠ ]");

    Card card2("Kh");
    EXPECT_EQ(card2.prettyString(), "[ K ♥ ]");

    Card card3("2c");
    EXPECT_EQ(card3.prettyString(), "[ 2 ♣ ]");
}

TEST(CardTest, BinaryString) {
    Card card("As");
    std::string binary = card.binaryString();
    EXPECT_FALSE(binary.empty());
    EXPECT_EQ(binary.length(), 35); // 32 bits + 3 spaces
}

TEST(CardTest, Equality) {
    Card card1("As");
    Card card2("As");
    Card card3("Kh");

    EXPECT_TRUE(card1 == card2);
    EXPECT_FALSE(card1 == card3);
    EXPECT_TRUE(card1 != card3);
}

TEST(CardTest, CardsFromStrings) {
    std::vector<std::string> card_strs = {"As", "Kh", "Qd", "Jc"};
    std::vector<Card> cards = cardsFromStrings(card_strs);

    EXPECT_EQ(cards.size(), 4);
    EXPECT_EQ(cards[0].toString(), "As");
    EXPECT_EQ(cards[1].toString(), "Kh");
    EXPECT_EQ(cards[2].toString(), "Qd");
    EXPECT_EQ(cards[3].toString(), "Jc");
}

TEST(CardTest, PrimeProductFromHand) {
    std::vector<Card> cards = cardsFromStrings({"As", "Ks", "Qs", "Js", "Ts"});
    int product = primeProductFromHand(cards);
    EXPECT_GT(product, 0);
}

TEST(CardTest, PrimeProductFromRankbits) {
    int rankbits = (1 << 12) | (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8); // AKQJT
    int product = primeProductFromRankbits(rankbits);
    EXPECT_GT(product, 0);
}

TEST(CardTest, PrettyPrintCards) {
    std::vector<Card> cards = cardsFromStrings({"As", "Kh", "Qd", "Jc"});
    std::string pretty = prettyPrintCards(cards);
    EXPECT_FALSE(pretty.empty());
    EXPECT_NE(pretty.find("♠"), std::string::npos);
    EXPECT_NE(pretty.find("♥"), std::string::npos);
    EXPECT_NE(pretty.find("♦"), std::string::npos);
    EXPECT_NE(pretty.find("♣"), std::string::npos);
} 