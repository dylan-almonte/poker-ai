#pragma once
#include "game.hpp"
#include "info_state.hpp"
#include "memory_buffer.hpp"
#include "cfr_network.hpp"
#include <memory>
#include <random>

class DeepCFR {
public:
    DeepCFR(int num_players, int num_actions, int state_size);
    
    // Train the Deep CFR model
    void train(int num_iterations, int num_traversals, int batch_size);
    
    // Get strategy at a given information state
    std::vector<float> getStrategy(const InfoState& info_state);
    
    // Save/load models
    void saveModels(const std::string& path);
    void loadModels(const std::string& path);
    
    // Add new member variables
    std::vector<float> iteration_weights;
    float alpha; // Learning rate for advantage networks
    
    // Add new methods
    void initializeWeights(int num_iterations);
    float computeIterationWeight(int t);
    
private:
    // External sampling MCCFR traversal
    std::vector<float> traverse(Game& game, int player_id, float reach_prob);
    
    // Compute advantages for a player at an information state
    std::vector<float> computeAdvantages(int player_id, const InfoState& info_state);
    
    // Compute strategy from advantages
    std::vector<float> computeStrategy(const std::vector<float>& advantages);
    
    int num_players;
    int num_actions;
    int state_size;
    
    std::unique_ptr<ValueNet> value_net;
    std::unique_ptr<StrategyNet> strategy_net;
    
    std::vector<std::unique_ptr<AdvantageMemory>> advantage_memories;
    std::unique_ptr<StrategyMemory> strategy_memory;
    
    std::mt19937 rng;
};