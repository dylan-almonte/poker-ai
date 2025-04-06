#pragma once
#include "game.hpp"
#include "player.hpp"
#include "deck.hpp"
#include "pot.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include <vector>
#include <memory>
#include <deque>
#include <random>

class TexasHoldEm {
private:
    int buyin_;
    int big_blind_;
    int small_blind_;
    int max_players_;
    
    std::vector<std::shared_ptr<Player>> players_;
    int btn_loc_;
    int bb_loc_;
    int sb_loc_;
    int current_player_;
    
    std::vector<std::shared_ptr<Pot>> pots_;
    std::unique_ptr<Deck> deck_;
    std::vector<Card> board_;
    std::unordered_map<int, std::vector<Card>> hands_;
    
    int last_raise_;
    bool raise_option_;
    int num_hands_;
    HandPhase::Phase hand_phase_;
    
    // Private helper methods
    void dealCards();
    void postBlinds();
    void moveBlinds();
    bool isValidAction(Action action) const;
    void handleAction(Action action);
    void settleHand();
    void setupPlayerQueue();
    void splitPot(int pot_id, int raised_level);
    void playerPost(int player_id, int amount);
    int getLastPotId() const;
    bool isHandOver() const;
    std::pair<ActionType, int> translateAllIn(ActionType action, int total = 0) const;
    int previousAllInSum() const;

public:
    TexasHoldEm(int buyin, int big_blind, int small_blind, int max_players = 9);
    
    void startHand();
    void takeAction(Action action);
    bool isHandComplete() const;
    bool isHandRunning() const;
    
    // Getters
    const std::vector<Card>& getBoard() const { return board_; }
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    int getCurrentPlayer() const { return current_player_; }
    HandPhase::Phase getPhase() const { return hand_phase_; }
    const std::vector<Card>& getHand(int player_id) const;
    
    // Player iteration helpers
    std::vector<int> getActivePlayers() const;
    int getNextActivePlayer(int from) const;
    
    // Betting helpers
    int chipsToCall(int player_id) const;
    int playerBetAmount(int player_id) const;
    int chipsAtStake(int player_id) const;
    int minRaise() const;
    
    // Validation
    std::pair<bool, std::string> validateMove(
        int player_id,
        ActionType action,
        int total = 0
    ) const;
}; 