#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "game_state.hpp"
#include "betting_round.hpp"
#include "hand_phase.hpp"

class Game {
private:
    std::vector<Action> action_history_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    std::vector<Card> board_;

    std::unique_ptr<BettingRound> betting_round_;
    Deck deck_;
    HandPhase::Phase phase_;
    int btn_loc_;
    int current_player_;

    int small_blind_;
    int big_blind_;
    int last_raise_ = 0;
    bool _isHandOver() const;

    void _takeAction(Action action);

    void _deal_cards();
    
    void _split_pot(int pot_idx, int raise_level);
    
    void _move_blinds();

    // returns rewards for each player
    void _settle_hand();
    



public:
    
    Game(int num_players, int starting_chips, int small_blind, int big_blind);
    
    // Core game flow methods
    GameState startHand(int btn_loc = -1);

    GameState takeAction(Action action);
    bool isHandOver() const;
    bool isHandComplete() const;
    
    // Getters
    const std::vector<Card>& getBoard() const { return board_; }
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    int getCurrentPlayer() const { return current_player_; }
    HandPhase::Phase getPhase() const { return phase_; }
    
    void printState() const;

    GameState getGameState() const {
        GameState state;
        state.num_players = players_.size();
        state.num_pots = pots_.size();
        state.current_player = current_player_;
        state.street = phase_;
        state.player_chips = std::vector<int>(players_.size(), 0);
        state.player_bets = std::vector<int>(players_.size(), 0);
        state.player_rewards = std::vector<int>(players_.size(), 0);
        state.player_states = std::vector<PlayerState>(players_.size(), PlayerState::OUT);
        for (size_t i = 0; i < players_.size(); ++i) {
            state.player_chips[i] = players_[i]->getChips();
            state.player_bets[i] = pots_.back()->get_player_amount(i);
            state.player_states[i] = players_[i]->getState();
        }
        state.board = std::vector<Card>(5, Card(-1));
        for (size_t i = 0; i < board_.size(); ++i) {
            state.board[i] = board_[i];
        }
        state.hole_cards = std::pair<Card, Card>(players_[current_player_]->getHand()[0], 
                                                 players_[current_player_]->getHand()[1]);
        state.hand_phase = phase_;
        return state;
    }
    // Game state helpers

    // Debug helper
}; 