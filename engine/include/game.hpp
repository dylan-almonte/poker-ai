#pragma once
#include <vector>
#include <memory>
#include <deque>
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action_type.hpp"

class Game {
private:
    std::vector<std::shared_ptr<Player>> players;
    std::vector<std::shared_ptr<Pot>> pots;
    std::vector<Card> board;
    Deck deck;
    
    int btn_loc;
    int current_player;
    HandPhase::Phase phase;
    
    int small_blind;
    int big_blind;
    
    void dealCards();
    void postBlinds();
    void moveBlinds();
    bool isValidAction(ActionType action, int amount) const;
    void handleAction(ActionType action, int amount);
    void settleHand();

public:
    Game(int num_players, int starting_chips, int small_blind, int big_blind);
    
    void startHand();
    void takeAction(ActionType action, int amount = 0);
    bool isHandComplete() const;
    
    // Getters
    const std::vector<Card>& getBoard() const { return board; }
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots; }
    int getCurrentPlayer() const { return current_player; }
    HandPhase::Phase getPhase() const { return phase; }
    
    // Player iteration helpers
    std::vector<int> getActivePlayers() const;
    int getNextActivePlayer(int from) const;
}; 