#pragma once

#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include "engine.hpp"
#include "neural_net.hpp"
#include "reservoir_buffer.hpp"
#include "info_state.hpp"

class DeepCFR {
private:
    // Neural networks for advantage estimation
    std::vector<std::shared_ptr<NeuralNet>> advantage_nets;
    
    // Neural network for the average policy
    std::shared_ptr<NeuralNet> strategy_net;
    
    // Reservoir buffers for advantage training
    std::vector<ReservoirBuffer<AdvantageMemory>> advantage_buffers;
    
    // Reservoir buffer for strategy training
    ReservoirBuffer<StrategyMemory> strategy_buffer;
    
    // Random number generator
    std::mt19937 rng;
    
    // Parameters
    int num_traversals;
    int num_players;
    float cfr_lr;
    float cfr_batch_size;
    float strategy_lr;
    float strategy_batch_size;
    
    // Helper methods
    float traverseCFR(const Game& state, int player_id, int iteration);
    std::vector<float> computeStrategy(const InfoState& info_state, int player_id);
    void updateAdvantageNet(int player_id);
    void updateStrategyNet();

public:
    DeepCFR(int num_players, 
            int num_traversals = 1000,
            float cfr_lr = 0.001,
            float cfr_batch_size = 128,
            float strategy_lr = 0.001,
            float strategy_batch_size = 128);
    
    // Train the Deep CFR agent
    void train(int iterations);
    
    // Get action probabilities for a given info state
    std::vector<float> getActionProbabilities(const InfoState& info_state);
    
    // Save and load models
    void saveModels(const std::string& path);
    void loadModels(const std::string& path);
}; 