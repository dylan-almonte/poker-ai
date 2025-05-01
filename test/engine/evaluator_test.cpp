#include <gtest/gtest.h>
#include "../../src/include/engine_v2/evaluator.hpp"
#include "../../src/include/engine_v2/card.hpp"

TEST(EvaluatorTest, EvaluateRoyalFlush) {
    std::vector<Card> hole = cardsFromStrings({"As", "Ks"});
    std::vector<Card> board = cardsFromStrings({"Qs", "Js", "Ts", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 1); // Straight Flush
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Straight Flush");
}

TEST(EvaluatorTest, EvaluateFourOfAKind) {
    std::vector<Card> hole = cardsFromStrings({"As", "Ac"});
    std::vector<Card> board = cardsFromStrings({"Ah", "Ad", "Ks", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 2); // Four of a Kind
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Four of a Kind");
}

TEST(EvaluatorTest, EvaluateFullHouse) {
    std::vector<Card> hole = cardsFromStrings({"As", "Ac"});
    std::vector<Card> board = cardsFromStrings({"Ah", "Ks", "Kd", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 3); // Full House
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Full House");
}

TEST(EvaluatorTest, EvaluateFlush) {
    std::vector<Card> hole = cardsFromStrings({"As", "2s"});
    std::vector<Card> board = cardsFromStrings({"3s", "4s", "6s", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 4); // Flush
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Flush");
}

TEST(EvaluatorTest, EvaluateStraight) {
    std::vector<Card> hole = cardsFromStrings({"As", "Kc"});
    std::vector<Card> board = cardsFromStrings({"Qh", "Jd", "Ts", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 5); // Straight
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Straight");
}

TEST(EvaluatorTest, EvaluateThreeOfAKind) {
    std::vector<Card> hole = cardsFromStrings({"As", "Ac"});
    std::vector<Card> board = cardsFromStrings({"Ah", "Ks", "Qd", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 6); // Three of a Kind
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Three of a Kind");
}

TEST(EvaluatorTest, EvaluateTwoPair) {
    std::vector<Card> hole = cardsFromStrings({"As", "Ac"});
    std::vector<Card> board = cardsFromStrings({"Ks", "Kd", "Qd", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 7); // Two Pair
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Two Pair");
}

TEST(EvaluatorTest, EvaluatePair) {
    std::vector<Card> hole = cardsFromStrings({"As", "Ac"});
    std::vector<Card> board = cardsFromStrings({"Ks", "Qd", "Jd", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 8); // Pair
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Pair");
}

TEST(EvaluatorTest, EvaluateHighCard) {
    std::vector<Card> hole = cardsFromStrings({"As", "Kc"});
    std::vector<Card> board = cardsFromStrings({"Qs", "Jd", "9d", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 9); // High Card
    EXPECT_EQ(Evaluator::rank_to_string(rank), "High Card");
}

TEST(EvaluatorTest, EvaluateWheelStraight) {
    std::vector<Card> hole = cardsFromStrings({"As", "2c"});
    std::vector<Card> board = cardsFromStrings({"3h", "4d", "5s", "6h", "7d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 5); // Straight
    EXPECT_EQ(Evaluator::rank_to_string(rank), "Straight");
}

TEST(EvaluatorTest, EvaluateRankPercentage) {
    // Royal Flush should be 100%
    std::vector<Card> royal_flush = cardsFromStrings({"As", "Ks", "Qs", "Js", "Ts"});
    int royal_rank = Evaluator::evaluate(royal_flush, {});
    EXPECT_FLOAT_EQ(Evaluator::get_five_card_rank_percentage(royal_rank), 1.0f);

    // High Card should be close to 0%
    std::vector<Card> high_card = cardsFromStrings({"2s", "3h", "4d", "5c", "7s"});
    int high_rank = Evaluator::evaluate(high_card, {});
    EXPECT_LT(Evaluator::get_five_card_rank_percentage(high_rank), 0.1f);
}

TEST(EvaluatorTest, EvaluateHandComparison) {
    // Royal Flush vs Four of a Kind
    std::vector<Card> royal_flush = cardsFromStrings({"As", "Ks", "Qs", "Js", "Ts"});
    std::vector<Card> four_kind = cardsFromStrings({"As", "Ac", "Ah", "Ad", "Ks"});
    
    int royal_rank = Evaluator::evaluate(royal_flush, {});
    int four_rank = Evaluator::evaluate(four_kind, {});
    
    EXPECT_LT(royal_rank, four_rank); // Royal Flush should be better

    // Four of a Kind vs Full House
    std::vector<Card> full_house = cardsFromStrings({"As", "Ac", "Ah", "Ks", "Kd"});
    int full_rank = Evaluator::evaluate(full_house, {});
    
    EXPECT_LT(four_rank, full_rank); // Four of a Kind should be better
}

TEST(EvaluatorTest, EvaluateSevenCards) {
    // Test with 7 cards (Texas Hold'em)
    std::vector<Card> hole = cardsFromStrings({"As", "Ks"});
    std::vector<Card> board = cardsFromStrings({"Qs", "Js", "Ts", "2h", "3d"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 1); // Should find the Royal Flush
}

TEST(EvaluatorTest, EvaluateMoreThanSevenCards) {
    // Test with more than 7 cards
    std::vector<Card> hole = cardsFromStrings({"As", "Ks"});
    std::vector<Card> board = cardsFromStrings({"Qs", "Js", "Ts", "2h", "3d", "4c", "5h"});
    int rank = Evaluator::evaluate(hole, board);
    EXPECT_EQ(Evaluator::get_rank_class(rank), 1); // Should find the Royal Flush
} 