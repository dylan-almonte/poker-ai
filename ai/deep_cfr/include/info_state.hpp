#pragma once

#include <vector>
#include <string>
#include "engine.hpp"


#define MAX_FEATURE_SIZE 500
// Class to represent an information state in poker
class InfoState {
private:
    int player_id_;
    std::vector<Card> hole_cards_;
    std::vector<Card> board_cards_;
    HandPhase::Phase phase_;
    std::vector<int> pot_sizes_;
    std::vector<int> player_stacks_;
    std::vector<PlayerState> player_states_;
    std::vector<Action> action_history_;

    float total_pot_size_ = 0.0f;

public:
    InfoState(int player_id,
              const std::vector<Card>& hole_cards,
              const std::vector<Card>& board_cards,
              HandPhase::Phase phase,
              const std::vector<int>& pot_sizes,
              const std::vector<int>& player_stacks,
              const std::vector<PlayerState>& player_states,
              const std::vector<Action>& action_history,
              float total_pot_size);

    // Create an InfoState from a Game object
    static InfoState fromGame(const Game& game, int player_id);

    // Convert to a feature vector for neural network input
    std::vector<float> toFeatureVector() const;

    // Get the number of legal actions in this state
    int getNumActions() const;

    // Get the legal actions in this state
    std::vector<Action> getLegalActions() const;

    // Get a string representation of the info state (for debugging)
    std::string toString() const;

    // Getters
    int getPlayerId() const { return player_id_; }
    const std::vector<Card>& getHoleCards() const { return hole_cards_; }
    const std::vector<Card>& getBoardCards() const { return board_cards_; }
    HandPhase::Phase getPhase() const { return phase_; }
};

