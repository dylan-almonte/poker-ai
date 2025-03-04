#pragma once

#include <vector>
#include <string>
#include "engine.hpp"

// Class to represent an information state in poker
class InfoState {
private:
    int player_id;
    std::vector<Card> hole_cards;
    std::vector<Card> board_cards;
    HandPhase::Phase phase;
    std::vector<int> pot_sizes;
    std::vector<int> player_stacks;
    std::vector<PlayerState> player_states;
    std::vector<std::pair<int, ActionType>> action_history;
    
public:
    InfoState(int player_id, 
              const std::vector<Card>& hole_cards,
              const std::vector<Card>& board_cards,
              HandPhase::Phase phase,
              const std::vector<int>& pot_sizes,
              const std::vector<int>& player_stacks,
              const std::vector<PlayerState>& player_states,
              const std::vector<std::pair<int, ActionType>>& action_history);
    
    // Create an InfoState from a Game object
    static InfoState fromGame(const Game& game, int player_id);
    
    // Convert to a feature vector for neural network input
    std::vector<float> toFeatureVector() const;
    
    // Get the number of legal actions in this state
    int getNumActions() const;
    
    // Get the legal actions in this state
    std::vector<ActionType> getLegalActions() const;
    
    // Get a string representation of the info state (for debugging)
    std::string toString() const;
    
    // Getters
    int getPlayerId() const { return player_id; }
    const std::vector<Card>& getHoleCards() const { return hole_cards; }
    const std::vector<Card>& getBoardCards() const { return board_cards; }
    HandPhase::Phase getPhase() const { return phase; }
}; 