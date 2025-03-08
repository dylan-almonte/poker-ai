#pragma once

#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include "engine.hpp"
#include "cfr_neural_net.hpp"
#include "reservoir_buffer.hpp"
#include "info_state.hpp"

class DeepCFR {
private:
    // Neural networks for advantage estimation (one per player)
    std::vector<std::shared_ptr<NeuralNet>> advantage_nets_;
    
    // Neural network for the average policy
    std::shared_ptr<NeuralNet> strategy_net_;
    
    // Reservoir buffers for advantage training (one per player)
    std::vector<ReservoirBuffer<AdvantageMemory>> advantage_buffers_;
    
    // Reservoir buffer for strategy training
    ReservoirBuffer<StrategyMemory> strategy_buffer_;
    
    // Random number generator
    std::mt19937 rng_;
    
    // Parameters
    int num_players_;
    int num_traversals_;
    float alpha_; // Linear weighting of iterations (typically 2.0)
    std::vector<float> iteration_weights_;
    
    // Helper methods
    float traverseCFR(Game& state, int traversing_player, int iteration, float reach_prob);
    std::vector<float> computeStrategy(const InfoState& info_state, int player_id);
    void updateAdvantageNet(int player_id, int batch_size);
    void updateStrategyNet(int batch_size);

public:
    DeepCFR(int num_players, 
            int num_traversals = 1000,
            float alpha = 2.0);
    
    // Train the Deep CFR agent
    void train(int iterations, int advantage_batch_size = 128, int strategy_batch_size = 128);
    
    // Get action probabilities for a given info state
    std::vector<float> getActionProbabilities(const InfoState& info_state);
    
    // Save and load models
    void saveModels(const std::string& path);
    void loadModels(const std::string& path);
}; 