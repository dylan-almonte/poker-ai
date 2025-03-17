#pragma once
#include <vector>
#include <memory>
#include <deque>
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"



class Game {
private:
    std::vector<Action> action_history_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    std::vector<Card> board_;
    Deck deck_;
    
    int btn_loc_;
    int current_player_;
    int last_player_; // last player to act in the current phase
    HandPhase::Phase phase_;
    
    int small_blind_;
    int big_blind_;
    
    void dealCards();
    void postBlinds();
    void moveBlinds();
    bool isValidAction(Action action) const;
    void handleAction(Action action);
    void setupPlayerQueue();
    Action translateAllIn(Action action);


public:

    Game(int num_players, int starting_chips, int small_blind, int big_blind);
    
    void startHand(int btn_loc = -1);
    void takeAction(Action action);
    bool isHandComplete() const;
    
    // Getters
    const std::vector<Card>& getBoard() const { return board_; }
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    int getCurrentPlayer() const { return current_player_; }
    HandPhase::Phase getPhase() const { return phase_; }
    bool isHandOver() const;
    void settleHand();
    
    // Add this new method
    void printState() const;
    
    // Player iteration helpers
    std::vector<int> getActivePlayers() const;
    int getNextActivePlayer(int from) const;

    float getPayoff(int player_idx) const;
    int getInitialStackTotal() const;
    
    // Get action history
    const std::vector<Action>& getActionHistory() const { return action_history_; }
    
}; 