#include <gtest/gtest.h>
#include "../../src/include/engine_v2/deck.hpp"

TEST(DeckTest, Constructor) {
    Deck deck;
    EXPECT_EQ(deck.size(), 52);
}

TEST(DeckTest, Shuffle) {
    Deck deck1;
    Deck deck2;
    
    // Both decks should start with the same order
    EXPECT_EQ(deck1.toString(), deck2.toString());
    
    // Shuffle one deck
    deck1.shuffle();
    
    // Decks should now be different (very small chance of being the same)
    EXPECT_NE(deck1.toString(), deck2.toString());
}

TEST(DeckTest, Draw) {
    Deck deck;
    
    // Draw one card
    std::vector<Card> cards = deck.draw();
    EXPECT_EQ(cards.size(), 1);
    EXPECT_EQ(deck.size(), 51);
    
    // Draw multiple cards
    cards = deck.draw(5);
    EXPECT_EQ(cards.size(), 5);
    EXPECT_EQ(deck.size(), 46);
}

TEST(DeckTest, DrawTooMany) {
    Deck deck;
    EXPECT_THROW(deck.draw(53), std::runtime_error);
}

TEST(DeckTest, DrawAll) {
    Deck deck;
    std::vector<Card> cards = deck.draw(52);
    EXPECT_EQ(cards.size(), 52);
    EXPECT_EQ(deck.size(), 0);
}

TEST(DeckTest, DrawEmpty) {
    Deck deck;
    deck.draw(52); // Empty the deck
    EXPECT_THROW(deck.draw(), std::runtime_error);
}

TEST(DeckTest, DrawUnique) {
    Deck deck;
    std::vector<Card> cards = deck.draw(52);
    
    // Check that all cards are unique
    for (size_t i = 0; i < cards.size(); ++i) {
        for (size_t j = i + 1; j < cards.size(); ++j) {
            EXPECT_NE(cards[i], cards[j]);
        }
    }
}

TEST(DeckTest, DrawOrder) {
    Deck deck1;
    Deck deck2;
    
    // Draw cards in same order from both decks
    std::vector<Card> cards1 = deck1.draw(5);
    std::vector<Card> cards2 = deck2.draw(5);
    
    // Cards should be in same order before shuffle
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(cards1[i], cards2[i]);
    }
    
    // Shuffle and draw again
    deck1.shuffle();
    deck2.shuffle();
    cards1 = deck1.draw(5);
    cards2 = deck2.draw(5);
    
    // Cards should be in different order after shuffle
    bool all_same = true;
    for (size_t i = 0; i < 5; ++i) {
        if (cards1[i] != cards2[i]) {
            all_same = false;
            break;
        }
    }
    EXPECT_FALSE(all_same);
}

TEST(DeckTest, FullDeck) {
    Deck deck;
    std::vector<Card> cards = deck.draw(52);
    
    // Check that all ranks and suits are present
    std::array<bool, 13> ranks = {false};
    std::array<bool, 4> suits = {false};
    
    for (const auto& card : cards) {
        ranks[card.getRank()] = true;
        suits[card.getSuit() >> 1] = true; // Convert suit to 0-3
    }
    
    // All ranks should be present
    for (bool rank : ranks) {
        EXPECT_TRUE(rank);
    }
    
    // All suits should be present
    for (bool suit : suits) {
        EXPECT_TRUE(suit);
    }
} 