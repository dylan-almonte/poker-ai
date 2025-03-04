#pragma once
#include "game.hpp"
#include "card.hpp"
#include "action_type.hpp"
#include <vector>
#include <string>

class InfoState {
public:
    InfoState(const Game& game, int player_id);
    
    // Get string representation of information state
    std::string toString() const;
    
    // Get vector representation for neural network input
    std::vector<float> getFeatures() const;
    
    // Get legal actions at this state
    std::vector<ActionType> getLegalActions() const;
    
    // Check if this is a terminal state
    bool isTerminal() const;
    
    // Get utility at terminal states
    float getUtility(int player_id) const;
    
private:
    int player_id;
    std::vector<Card> hole_cards;
    std::vector<Card> board_cards;
    HandPhase::Phase phase;
    int pot_size;
    int to_call;
    std::vector<int> betting_history;
    int num_players;
    int position;
};