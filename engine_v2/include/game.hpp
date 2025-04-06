#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "state_machines.hpp"
#include "hand_phase.hpp"

class Game {
private:
    std::vector<Action> action_history_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    std::vector<Card> board_;

    BettingRound betting_round_;
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

    void _settle_hand();
    



public:
    Game(int num_players, int starting_chips, int small_blind, int big_blind);
    
    // Core game flow methods
    void startHand(int btn_loc = -1);

    void takeAction(Action action);
    bool isHandOver() const;
    bool isHandComplete() const;
    
    // Getters
    const std::vector<Card>& getBoard() const { return board_; }
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    int getCurrentPlayer() const { return current_player_; }
    HandPhase::Phase getPhase() const { return phase_; }
    
    
    // Game state helpers

    // Debug helper
    void printState() const;
}; 