#pragma once

#include "engine.hpp"
#include "deep_cfr.hpp"
#include "info_state.hpp"
#include <memory>
#include <random>

class DeepCFRPlayer : public Player {
private:
    std::shared_ptr<DeepCFR> deep_cfr_;
    std::mt19937 rng_;
    bool explore_;
    float explore_prob_;

public:
    DeepCFRPlayer(int id, const std::string& name, int chips, 
                  std::shared_ptr<DeepCFR> deep_cfr,
                  bool explore = false, 
                  float explore_prob = 0.05)
        : Player(id, name, chips),
          deep_cfr_(deep_cfr),
          rng_(std::random_device{}()),
          explore_(explore),
          explore_prob_(explore_prob) {}

    void takeAction(const Game& game, Action& action, int& amount) {
        // Create info state from game
        InfoState info_state = InfoState::fromGame(game, getId());
        
        // Get action probabilities from Deep CFR
        std::vector<float> probs = deep_cfr_->getActionProbabilities(info_state);
        std::vector<Action> legal_actions = info_state.getLegalActions();
        
        // Exploration: with small probability, choose a random action
        if (explore_ && std::uniform_real_distribution<float>(0, 1)(rng_) < explore_prob_) {
            int random_idx = std::uniform_int_distribution<int>(0, legal_actions.size() - 1)(rng_);
            action = legal_actions[random_idx];
        } else {
            // Otherwise, sample from the strategy
            float r = std::uniform_real_distribution<float>(0, 1)(rng_);
            float cumulative_prob = 0.0f;
            
            for (size_t i = 0; i < legal_actions.size(); i++) {
                cumulative_prob += probs[i];
                if (r < cumulative_prob) {
                    action = legal_actions[i];
                    break;
                }
            }
        }
        
        // Set amount based on action type
        // This is a simplified version - in a real implementation, you would want
        // to have the neural network also predict bet sizes
        if (action.getActionType() == ActionType::RAISE) {
            // Simple heuristic: raise 2x the current bet
            amount = 2 * game.getPots().back()->chips_to_call(getId());
        } else if (action.getActionType() == ActionType::ALL_IN) {
            amount = getChips();
        } else {
            // For CALL, CHECK, FOLD, the amount is determined by the game engine
            amount = 0;
        }
    }
}; 