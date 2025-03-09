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
    int last_raise_ = 0;
    
    void _dealCards();
    void _postBlinds();

    /**
     * Let a player post the given amount and sets the corresponding board state
     * (i.e. makes other player states TO_CALL, sets ALL_IN). Also handles all
     * pots (i.e. split pots).
     * 
     * @param player_idx The index of the player to post the amount
     * @param amount The amount to post
     */
    void _postPlayerBets(int player_idx, int amount);

    /**
     * 
     */
    void _splitPot(int pot_idx, int raise_level);

    void _preHandSetup();
    void _bettingRound(HandPhase::Phase phase);
    void _settleHand();

    void _moveBlinds();
    bool _isValidAction(Action action) const;
    void _handleAction(Action action);
    void _handlePhaseChange();
    void _setupPlayerQueue();
    Action _translateAllIn(Action action);


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
    int getTotalToCall(int player_id) const;
    // Get action history
    const std::vector<Action>& getActionHistory() const { return action_history_; }
    
}; 