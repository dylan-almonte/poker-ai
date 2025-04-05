#pragma once
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

    int small_blind_;
    int big_blind_;
    int last_raise_ = 0;
    bool _isHandOver() const;
    void _takeAction(Action action);

    


    void _dealCards() {
        deck_ = Deck(); // New shuffled deck

        // Deal 2 cards to each player
        for (int i = 0; i < 2; i++) {
            for (auto& player : players_) {
                if (player->getState() != PlayerState::SKIP) {
                    auto cards = deck_.draw(1);
                    player->addCard(cards[0]);
                }
                }
            }
    }
    void _postPlayerBets(int player_idx, int amount);
    void _splitPot(int pot_idx, int raise_level);
    void _moveBlinds();
    void _settleHand();
    void _handleAction(Action action);
    bool _isValidAction(Action action) const;
    Action _translateAllIn(Action action);

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
    
    
    // Game state helpers
    float getPayoff(int player_idx) const;
    int getInitialStackTotal() const;
    int getTotalToCall(int player_id) const;
    
    // Debug helper
    void printState() const;
}; 