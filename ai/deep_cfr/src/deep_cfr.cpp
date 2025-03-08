#include "deep_cfr.hpp"
#include <algorithm>
#include <numeric>
#include <iostream>
#include <filesystem>

DeepCFR::DeepCFR(int num_players, int num_traversals, float alpha)
    : strategy_buffer(1000000),
    rng(std::random_device{}()),
    num_players(num_players),
    num_traversals(num_traversals),
    alpha(alpha) { // Initialize strategy_buffer with capacity

    // Initialize neural networks
    const int input_size = 500;  // Size of the feature vector for poker states
    const int hidden_size = 256;
    const int output_size = 5;   // Number of possible actions in poker

    // Create advantage networks for each player
    for (int i = 0; i < num_players; i++) {
        advantage_nets.push_back(std::make_shared<NeuralNet>(
            input_size, hidden_size, output_size, 0.001)); // Learning rate 0.001
    }

    // Create strategy network
    strategy_net = std::make_shared<NeuralNet>(
        input_size, hidden_size, output_size, 0.001); // Learning rate 0.001

    // Initialize reservoir buffers
    const int buffer_size = 1000000;  // 1M samples as in the paper
    for (int i = 0; i < num_players; i++) {
        advantage_buffers.push_back(ReservoirBuffer<AdvantageMemory>(buffer_size));
    }
}

void DeepCFR::train(int iterations, int advantage_batch_size, int strategy_batch_size) {
    // Initialize iteration weights
    iteration_weights.resize(iterations);
    for (int i = 0; i < iterations; i++) {
        iteration_weights[i] = std::pow(i + 1, alpha);
    }

    for (int iter = 0; iter < iterations; iter++) {
        std::cout << "Iteration " << iter + 1 << "/" << iterations << std::endl;

        // For each player
        for (int player_id = 0; player_id < num_players; player_id++) {
            std::cout << "  Traversals for player " << player_id << std::endl;

            // Perform CFR traversals
            for (int t = 0; t < num_traversals; t++) {
                if (t % 100 == 0) {
                    std::cout << "    Traversal " << t << "/" << num_traversals << std::endl;
                }

                // Create a new game instance
                Game game(num_players, 1000, 10, 20);  // 6 players, 1000 chips, 10/20 blinds
                game.startHand();

                // Perform CFR traversal
                traverseCFR(game, player_id, iter, 1.0);
            }

            // Update advantage network for this player
            updateAdvantageNet(player_id, advantage_batch_size);
        }

        // Collect strategy data
        for (int t = 0; t < num_traversals; t++) {
            // Create a new game instance
            Game game(num_players, 1000, 10, 20);
            game.startHand();

            // Random player for this traversal
            int player_id = std::uniform_int_distribution<>(0, num_players - 1)(rng);
            traverseCFR(game, player_id, iter, 1.0);
        }

        // Update strategy network
        updateStrategyNet(strategy_batch_size);

        // Save models periodically
        if ((iter + 1) % 10 == 0 || iter == iterations - 1) {
            saveModels("models/iter_" + std::to_string(iter + 1));
        }
    }
}

float DeepCFR::traverseCFR(Game& game, int traversing_player, int iteration, float reach_prob) {
    // If the game is over, return the utility for the player
    if (game.isHandComplete()) {
        float payoff = game.getPayoff(traversing_player);
        // Normalize the payoff to [-1, 1] range by dividing by the maximum possible win/loss
        // In poker, this would typically be the maximum amount a player could win in the hand
        float max_possible_win = game.getInitialStackTotal(); // Sum of all players' initial stacks
        return payoff / max_possible_win; // This will give us a value between -1 and 1
    }

    // Get current player
    int current_player = game.getCurrentPlayer();

    // Create info state for the current player
    InfoState info_state = InfoState::fromGame(game, current_player);
    std::vector<Action> legal_actions = info_state.getLegalActions();

    // If it's not the traversing player's turn, use current strategy to sample an action
    if (current_player != traversing_player) {
        std::vector<float> strategy = computeStrategy(info_state, current_player);

        // Sample an action according to the strategy
        float r = std::uniform_real_distribution<float>(0, 1)(rng);
        float cumulative_prob = 0.0f;
        Action chosen_action = legal_actions[0];

        for (size_t i = 0; i < legal_actions.size(); i++) {
            cumulative_prob += strategy[i];
            if (r < cumulative_prob) {
                chosen_action = legal_actions[i];
                break;
            }
        }

        // Apply the chosen action and continue
        Game next_state = game;
        if (chosen_action.getActionType() == ActionType::RAISE) {
            next_state.takeAction(chosen_action);
        } else {
            next_state.takeAction(chosen_action);
        }
        return traverseCFR(next_state, traversing_player, iteration, reach_prob);
    }

    // If it's the traversing player's turn, compute counterfactual values for each action
    std::vector<float> cf_values(legal_actions.size(), 0.0f);

    // Compute strategy from regrets (using advantage network)
    std::vector<float> strategy = computeStrategy(info_state, traversing_player);

    // Record the strategy in the strategy buffer with iteration weight
    strategy_buffer.add(StrategyMemory(info_state, strategy, iteration_weights[iteration]));

    // Compute counterfactual values for each action
    float cf_value_sum = 0.0f;
    for (size_t i = 0; i < legal_actions.size(); i++) {
        Game next_state = game;
        next_state.takeAction(legal_actions[i]);
        cf_values[i] = traverseCFR(next_state, traversing_player, iteration, reach_prob);
        cf_value_sum += strategy[i] * cf_values[i];
    }

    // Compute regrets
    std::vector<float> regrets(legal_actions.size(), 0.0f);
    for (size_t i = 0; i < legal_actions.size(); i++) {
        regrets[i] = cf_values[i] - cf_value_sum;
    }

    // Add to advantage buffer with reach probability as weight
    advantage_buffers[traversing_player].add(AdvantageMemory(info_state, regrets, reach_prob));

    return cf_value_sum;
}

std::vector<float> DeepCFR::computeStrategy(const InfoState& info_state, int player_id) {
    // Convert info state to feature vector
    std::vector<float> features = info_state.toFeatureVector();

    // Get advantages from advantage network
    std::vector<float> advantages = advantage_nets[player_id]->predict(features);
    std::vector<Action> legal_actions = info_state.getLegalActions();

    // Convert advantages to strategy using regret matching
    std::vector<float> strategy(legal_actions.size(), 0.0f);
    float regret_sum = 0.0f;

    // Sum positive regrets
    for (size_t i = 0; i < legal_actions.size(); i++) {
        advantages[i] = std::max(advantages[i], 0.0f);
        regret_sum += advantages[i];
    }

    // Normalize to get strategy
    if (regret_sum > 0.0f) {
        for (size_t i = 0; i < legal_actions.size(); i++) {
            strategy[i] = advantages[i] / regret_sum;
        }
    } else {
        // Uniform strategy if all advantages are negative or zero
        float uniform_prob = 1.0f / legal_actions.size();
        std::fill(strategy.begin(), strategy.end(), uniform_prob);
    }

    return strategy;
}

void DeepCFR::updateAdvantageNet(int player_id, int batch_size) {
    if (advantage_buffers[player_id].size() < static_cast<size_t>(batch_size)) {
        std::cout << "Not enough samples to train advantage net for player " << player_id << std::endl;
        return;
    }

    std::cout << "Training advantage network for player " << player_id << std::endl;

    // Sample batch from buffer
    auto batch = advantage_buffers[player_id].sample(static_cast<size_t>(batch_size));

    // Prepare features and targets
    std::vector<std::vector<float>> features_batch;
    std::vector<std::vector<float>> targets_batch;

    for (const auto& memory : batch) {
        features_batch.push_back(memory.info_state.toFeatureVector());
        targets_batch.push_back(memory.advantages);
    }

    // Train the network
    float loss = advantage_nets[player_id]->train(features_batch, targets_batch, batch_size);
    std::cout << "  Loss: " << loss << std::endl;
}

void DeepCFR::updateStrategyNet(int batch_size) {
    if (strategy_buffer.size() < static_cast<size_t>(batch_size)) {
        std::cout << "Not enough samples to train strategy net" << std::endl;
        return;
    }

    std::cout << "Training strategy network" << std::endl;

    // Sample batch from buffer
    auto batch = strategy_buffer.sample(static_cast<size_t>(batch_size));

    // Prepare features and targets
    std::vector<std::vector<float>> features_batch;
    std::vector<std::vector<float>> targets_batch;

    for (const auto& memory : batch) {
        features_batch.push_back(memory.info_state.toFeatureVector());
        targets_batch.push_back(memory.strategy);
    }

    // Train the network
    float loss = strategy_net->train(features_batch, targets_batch, batch_size);
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