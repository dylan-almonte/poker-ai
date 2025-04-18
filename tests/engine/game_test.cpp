#include <gtest/gtest.h>
#include "engine.hpp"

#ifndef NDEBUG
#define DEBUG_PRINT_GAME(game) game->printState()
#else
#define DEBUG_PRINT_GAME(game) ((void)0)
#endif


class GameTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup: 6 players, 1000 starting chips, 5/10 blinds
        game = std::make_unique<Game>(6, 1000, 5, 10);
        headsup_game = std::make_unique<Game>(2, 1000, 5, 10);
    }

    std::unique_ptr<Game> game;
    std::unique_ptr<Game> headsup_game;
};

TEST_F(GameTest, GameInitialization) {
    // Test initial game state
    ASSERT_EQ(game->getPlayers().size(), 6);
    ASSERT_EQ(game->getBoard().size(), 0);
    ASSERT_EQ(game->getPhase(), HandPhase::Phase::PREFLOP);

    // Verify all players have correct starting chips
    for (const auto& player : game->getPlayers()) {
        ASSERT_EQ(player->getChips(), 1000);
        ASSERT_EQ(player->getState(), PlayerState::IN);
    }
}

TEST_F(GameTest, StartHandPostsBlinds) {
    game->startHand(0);

    // After startHand(0), we should be in PREFLOP
    ASSERT_EQ(game->getPhase(), HandPhase::PREFLOP);

    // Verify blinds are posted correctly
    // Small blind (player 1)
    ASSERT_EQ(game->getPlayers()[1]->getChips(), 995);  // 1000 - 5
    // Big blind (player 2)
    ASSERT_EQ(game->getPlayers()[2]->getChips(), 990);  // 1000 - 10
}

TEST_F(GameTest, ValidActionSequence) {
    game->startHand(0);

    // First player to act is UTG (player 3)
    ASSERT_EQ(game->getCurrentPlayer(), 3);

    // UTG calls
    // DEBUG_PRINT_GAME(game);
    game->takeAction(Action(ActionType::CALL, 3, 10));
    // DEBUG_PRINT_GAME(game);
    ASSERT_EQ(game->getPlayers()[3]->getChips(), 990);

    // MP raises
    //  DEBUG_PRINT_GAME(game);
    game->takeAction(Action(ActionType::RAISE, 4, 30));
    // DEBUG_PRINT_GAME(game);
    ASSERT_EQ(game->getPlayers()[4]->getChips(), 970);

    // Verify action history
    // const auto& history = game->getActionHistory();
    // // ASSERT_EQ(history.size(), 2);
    // ASSERT_EQ(history[0].getActionType(), ActionType::CALL);
    // ASSERT_EQ(history[1].getActionType(), ActionType::RAISE);
}

TEST_F(GameTest, PlayerFolding) {
    game->startHand(0);

    // Player 3 folds
    game->takeAction(Action(ActionType::FOLD, 3, 0));

    // Verify player state
    ASSERT_EQ(game->getPlayers()[3]->getState(), PlayerState::OUT);
    ASSERT_FALSE(game->getPlayers()[3]->isActive());

    // Verify next active player
    ASSERT_EQ(game->getCurrentPlayer(), 4);
}

TEST_F(GameTest, AllInAction) {
    game->startHand(0);

    // Player 3 goes all-in
    game->takeAction(Action(ActionType::ALL_IN));

    // Verify player state and chips
    ASSERT_EQ(game->getPlayers()[3]->getState(), PlayerState::ALL_IN);
    ASSERT_EQ(game->getPlayers()[3]->getChips(), 0);
    ASSERT_TRUE(game->getPlayers()[3]->isAllIn());
}

TEST_F(GameTest, HandProgression) {
    game->startHand(0);

    // Simulate all players calling
    for (int i = 3; i <= 5; i++) {
        game->takeAction(Action(ActionType::CALL));
    }
    game->takeAction(Action(ActionType::CALL));
    game->takeAction(Action(ActionType::CALL));  // SB completes
    DEBUG_PRINT_GAME(game);
    game->takeAction(Action(ActionType::CHECK));  // BB checks
    // Should now be on the flop
    DEBUG_PRINT_GAME(game);
    ASSERT_EQ(game->getPhase(), HandPhase::FLOP);
    ASSERT_EQ(game->getBoard().size(), 3);
    for (int i = 0; i < 6; i++) {
        game->takeAction(Action(ActionType::CHECK));
    }
    ASSERT_EQ(game->getPhase(), HandPhase::TURN);
    ASSERT_EQ(game->getBoard().size(), 4);
    game->takeAction(Action(ActionType::CHECK));

}

TEST_F(GameTest, ALL_IN_ACTION) {
    GameState state;
    state = headsup_game->startHand(0);
    state.print();
    // DEBUG_PRINT_GAME(headsup_game);
    state = headsup_game->takeAction(Action(ActionType::ALL_IN));
    // DEBUG_PRINT_GAME(headsup_game);
    state = headsup_game->takeAction(Action(ActionType::ALL_IN));
    // DEBUG_PRINT_GAME(headsup_game);
    ASSERT_EQ(state.street, HandPhase::Phase::SETTLE);
    ASSERT_EQ(state.is_terminal, true);
    ASSERT_EQ(state.player_rewards[0], -state.player_rewards[1]);
    state.print();
    // ASSERT_EQ(game->getPlayers()[3]->getState(), PlayerState::ALL_IN);
    // ASSERT_EQ(game->getPlayers()[3]->getChips(), 0);
    // ASSERT_TRUE(game->getPlayers()[3]->isAllIn());
}

