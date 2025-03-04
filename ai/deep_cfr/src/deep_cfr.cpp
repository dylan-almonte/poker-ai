#include "deep_cfr.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <filesystem>

DeepCFR::DeepCFR(int num_players, 
                 int num_traversals,
                 float cfr_lr,
                 float cfr_batch_size,
                 float strategy_lr,
                 float strategy_batch_size)
    : num_players(num_players),
      num_traversals(num_traversals),
      cfr_lr(cfr_lr),
      cfr_batch_size(cfr_batch_size),
      strategy_lr(strategy_lr),
      strategy_batch_size(strategy_batch_size),
      rng(std::random_device{}()) {
    
    // Initialize neural networks
    const int input_size = 500;  // Size of the feature vector for poker states
    const int hidden_size = 256;
    const int output_size = 5;   // Number of possible actions in poker
    
    // Create advantage networks for each player
    for (int i = 0; i < num_players; i++) {
        advantage_nets.push_back(std::make_shared<NeuralNet>(
            input_size, hidden_size, output_size, cfr_lr));
    }
    
    // Create strategy network
    strategy_net = std::make_shared<NeuralNet>(
        input_size, hidden_size, output_size, strategy_lr);
    
    // Initialize reservoir buffers
    const int buffer_size = 1000000;  // 1M samples
    for (int i = 0; i < num_players; i++) {
        advantage_buffers.push_back(ReservoirBuffer<AdvantageMemory>(buffer_size));
    }
    strategy_buffer = ReservoirBuffer<StrategyMemory>(buffer_size);
}

void DeepCFR::train(int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        std::cout << "Iteration " << iter + 1 << "/" << iterations << std::endl;
        
        // Create a new game instance
        Game game(num_players, 1000, 10, 20);  // 6 players, 1000 chips, 10/20 blinds
        
        // Perform CFR traversals for each player
        for (int player_id = 0; player_id < num_players; player_id++) {
            std::cout << "  Traversals for player " << player_id << std::endl;
            
            for (int t = 0; t < num_traversals; t++) {
                if (t % 100 == 0) {
                    std::cout << "    Traversal " << t << "/" << num_traversals << std::endl;
                }
                
                // Reset the game
                game = Game(num_players, 1000, 10, 20);
                game.startHand();
                
                // Perform CFR traversal
                traverseCFR(game, player_id, iter);
            }
            
            // Update advantage network for this player
            updateAdvantageNet(player_id);
        }
        
        // Update strategy network
        updateStrategyNet();
        
        // Save models periodically
        if ((iter + 1) % 10 == 0 || iter == iterations - 1) {
            saveModels("models/iter_" + std::to_string(iter + 1));
        }
    }
}

float DeepCFR::traverseCFR(const Game& state, int player_id, int iteration) {
    // If the game is over, return the utility for the player
    if (state.isHandComplete()) {
        // TODO: Implement utility calculation based on the game outcome
        return 0.0f;
    }
    
    // Get current player
    int current_player = state.getCurrentPlayer();
    
    // If it's a chance node (dealing cards), sample one outcome and continue
    if (state.getPhase() == HandPhase::Phase::PREHAND) {
        Game next_state = state;
        next_state.startHand();
        return traverseCFR(next_state, player_id, iteration);
    }
    
    // Create info state for the current player
    InfoState info_state = InfoState::fromGame(state, current_player);
    
    // If it's not the traversing player's turn, use current strategy to sample an action
    if (current_player != player_id) {
        std::vector<float> strategy = computeStrategy(info_state, current_player);
        std::vector<ActionType> legal_actions = info_state.getLegalActions();
        
        // Sample an action according to the strategy
        float r = std::uniform_real_distribution<float>(0, 1)(rng);
        float cumulative_prob = 0.0f;
        ActionType chosen_action = legal_actions[0];
        
        for (size_t i = 0; i < legal_actions.size(); i++) {
            cumulative_prob += strategy[i];
            if (r < cumulative_prob) {
                chosen_action = legal_actions[i];
                break;
            }
        }
        
        // Apply the chosen action and continue
        Game next_state = state;
        next_state.takeAction(chosen_action);
        return traverseCFR(next_state, player_id, iteration);
    }
    
    // If it's the traversing player's turn, compute counterfactual values for each action
    std::vector<ActionType> legal_actions = info_state.getLegalActions();
    std::vector<float> cf_values(legal_actions.size(), 0.0f);
    std::vector<float> regrets(legal_actions.size(), 0.0f);
    
    // Compute strategy from regrets (using advantage network)
    std::vector<float> strategy = computeStrategy(info_state, player_id);
    
    // Record the strategy in the strategy buffer
    strategy_buffer.add(StrategyMemory(info_state, strategy, iteration));
    
    // Compute counterfactual values for each action
    float cf_value_sum = 0.0f;
    for (size_t i = 0; i < legal_actions.size(); i++) {
        Game next_state = state;
        next_state.takeAction(legal_actions[i]);
        cf_values[i] = traverseCFR(next_state, player_id, iteration);
        cf_value_sum += strategy[i] * cf_values[i];
    }
    
    // Compute regrets
    for (size_t i = 0; i < legal_actions.size(); i++) {
        regrets[i] = cf_values[i] - cf_value_sum;
    }
    
    // Add to advantage buffer
    advantage_buffers[player_id].add(AdvantageMemory(info_state, regrets, iteration));
    
    return cf_value_sum;
}

std::vector<float> DeepCFR::computeStrategy(const InfoState& info_state, int player_id) {
    // Convert info state to feature vector
    std::vector<float> features = info_state.toFeatureVector();
    
    // Get regrets from advantage network
    std::vector<float> regrets = advantage_nets[player_id]->predict(features);
    
    // Convert regrets to strategy using regret matching
    std::vector<float> strategy(regrets.size(), 0.0f);
    float regret_sum = 0.0f;
    
    // Sum positive regrets
    for (size_t i = 0; i < regrets.size(); i++) {
        regrets[i] = std::max(regrets[i], 0.0f);
        regret_sum += regrets[i];
    }
    
    // Normalize to get strategy
    if (regret_sum > 0.0f) {
        for (size_t i = 0; i < strategy.size(); i++) {
            strategy[i] = regrets[i] / regret_sum;
        }
    } else {
        // Uniform strategy if all regrets are negative or zero
        float uniform_prob = 1.0f / strategy.size();
        std::fill(strategy.begin(), strategy.end(), uniform_prob);
    }
    
    return strategy;
}

void DeepCFR::updateAdvantageNet(int player_id) {
    if (advantage_buffers[player_id].size() < cfr_batch_size) {
        std::cout << "Not enough samples to train advantage net for player " << player_id << std::endl;
        return;
    }
    
    std::cout << "Training advantage network for player " << player_id << std::endl;
    
    // Sample batch from buffer
    auto batch = advantage_buffers[player_id].sample(cfr_batch_size);
    
    // Prepare features and targets
    std::vector<std::vector<float>> features_batch;
    std::vector<std::vector<float>> targets_batch;
    
    for (const auto& memory : batch) {
        features_batch.push_back(memory.info_state.toFeatureVector());
        targets_batch.push_back(memory.advantages);
    }
    
    // Train the network
    float loss = advantage_nets[player_id]->train(features_batch, targets_batch, cfr_batch_size);
    std::cout << "  Loss: " << loss << std::endl;
}

void DeepCFR::updateStrategyNet() {
    if (strategy_buffer.size() < strategy_batch_size) {
        std::cout << "Not enough samples to train strategy net" << std::endl;
        return;
    }
    
    std::cout << "Training strategy network" << std::endl;
    
    // Sample batch from buffer
    auto batch = strategy_buffer.sample(strategy_batch_size);
    
    // Prepare features and targets
    std::vector<std::vector<float>> features_batch;
    std::vector<std::vector<float>> targets_batch;
    
    for (const auto& memory : batch) {
        features_batch.push_back(memory.info_state.toFeatureVector());
        targets_batch.push_back(memory.strategy);
    }
    
    // Train the network
    float loss = strategy_net->train(features_batch, targets_batch, strategy_batch_size);
    std::cout << "  Loss: " << loss << std::endl;
}

std::vector<float> DeepCFR::getActionProbabilities(const InfoState& info_state) {
    // Use the strategy network to get action probabilities
    return strategy_net->predict(info_state.toFeatureVector());
}

void DeepCFR::saveModels(const std::string& path) {
    // Create directory if it doesn't exist
    std::filesystem::create_directories(path);
    
    // Save advantage networks
    for (int i = 0; i < num_players; i++) {
        advantage_nets[i]->save(path + "/advantage_net_" + std::to_string(i) + ".pt");
    }
    
    // Save strategy network
    strategy_net->save(path + "/strategy_net.pt");
}

void DeepCFR::loadModels(const std::string& path) {
    // Load advantage networks
    for (int i = 0; i < num_players; i++) {
        advantage_nets[i]->load(path + "/advantage_net_" + std::to_string(i) + ".pt");
    }
    
    // Load strategy network
    strategy_net->load(path + "/strategy_net.pt");
} 