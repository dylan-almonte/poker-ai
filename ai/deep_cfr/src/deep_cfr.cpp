#include "deep_cfr.hpp"
#include <algorithm>
#include <numeric>

DeepCFR::DeepCFR(int num_players, int num_actions, int state_size)
    : num_players(num_players), num_actions(num_actions), state_size(state_size) {

    // Initialize networks
    value_net = std::make_unique<ValueNet>(num_players, state_size, num_actions);
    strategy_net = std::make_unique<StrategyNet>(state_size, num_actions);

    // Initialize memory buffers
    for (int i = 0; i < num_players; i++) {
        advantage_memories.push_back(std::make_unique<AdvantageMemory>(1000000)); // 1M capacity
    }
    strategy_memory = std::make_unique<StrategyMemory>(1000000);

    // Initialize random number generator
    std::random_device rd;
    rng = std::mt19937(rd());
}

void DeepCFR::train(int num_iterations, int num_traversals, int batch_size) {
    initializeWeights(num_iterations);

    for (int iter = 0; iter < num_iterations; iter++) {
        float weight = computeIterationWeight(iter);

        // For each player
        for (int player = 0; player < num_players; player++) {
            // Perform traversals to collect advantage data
            for (int t = 0; t < num_traversals; t++) {
                Game game(num_players, 1000, 10, 20);
                game.startHand();
                traverse(game, player, 1.0, weight);
            }

            // Train value network for current player with weighted samples
            auto batch = advantage_memories[player]->sampleWeighted(batch_size);
            value_net->train(player, batch);
        }

        // Collect strategy data
        for (int t = 0; t < num_traversals; t++) {
            Game game(num_players, 1000, 10, 20);
            game.startHand();

            // Random player for this traversal
            int player = std::uniform_int_distribution<>(0, num_players - 1)(rng);
            traverse(game, player, 1.0);
        }

        // Train strategy network
        auto batch = strategy_memory->sample(batch_size);
        strategy_net->train(batch);
    }
}

std::vector<float> DeepCFR::traverse(Game& game, int player_id, float reach_prob) {
    if (game.isHandComplete()) {
        // Terminal state - return utilities
        std::vector<float> utilities(num_actions, 0.0f);
        // Calculate utility based on pot distribution
        // This depends on your Game implementation
        return utilities;
    }

    int current_player = game.getCurrentPlayer();
    InfoState info_state(game, current_player);

    if (current_player != player_id) {
        // Opponent node - sample action according to current strategy
        auto strategy = getStrategy(info_state);
        auto legal_actions = info_state.getLegalActions();

        // Sample action based on strategy
        std::discrete_distribution<> dist(strategy.begin(), strategy.end());
        int action_idx = dist(rng);
        ActionType action = legal_actions[action_idx];

        // Clone game and apply action
        Game next_game = game; // Assuming Game has copy constructor
        next_game.takeAction(action);

        // Recurse
        return traverse(next_game, player_id, reach_prob);
    } else {
        // Current player node - compute advantages
        auto advantages = computeAdvantages(player_id, info_state);
        auto legal_actions = info_state.getLegalActions();

        // Compute strategy from advantages
        auto strategy = computeStrategy(advantages);

        // Add to strategy memory
        strategy_memory->add(info_state.toString(), strategy);

        // Sample action and recurse
        std::vector<float> action_values(num_actions, 0.0f);

        for (size_t a = 0; a < legal_actions.size(); a++) {
            if (advantages[a] > 0) { // Only explore positive advantage actions
                Game next_game = game;
                next_game.takeAction(legal_actions[a]);

                auto action_value = traverse(next_game, player_id, reach_prob * strategy[a]);
                action_values[a] = action_value[a];

                // Compute instantaneous regret
                float regret = action_value[a] -
                    std::inner_product(strategy.begin(), strategy.end(), action_values.begin(), 0.0f);

                // Add to advantage memory
                addToMemory(player_id, info_state.toString(), { regret }, 1.0f);
            }
        }

        return action_values;
    }
}

std::vector<float> DeepCFR::computeAdvantages(int player_id, const InfoState& info_state) {
    return value_net->predict(player_id, info_state.getFeatures());
}

std::vector<float> DeepCFR::computeStrategy(const std::vector<float>& advantages) {
    std::vector<float> strategy(advantages.size());

    // Find max advantage for numerical stability
    float max_advantage = *std::max_element(advantages.begin(), advantages.end());

    // Compute exp(advantage) and sum
    float sum = 0.0f;
    for (size_t i = 0; i < advantages.size(); i++) {
        strategy[i] = std::exp(advantages[i] - max_advantage);
        sum += strategy[i];
    }

    // Normalize to get final strategy
    if (sum > 0) {
        for (float& prob : strategy) {
            prob /= sum;
        }
    } else {
        // If all advantages are very negative, use uniform strategy
        float uniform_prob = 1.0f / advantages.size();
        std::fill(strategy.begin(), strategy.end(), uniform_prob);
    }

    return strategy;
}

std::vector<float> DeepCFR::getStrategy(const InfoState& info_state) {
    return strategy_net->predict(info_state.getFeatures());
}

void DeepCFR::addToMemory(int player_id, const std::string& info_state,
                          const std::vector<float>& advantages, float weight) {
    auto& memory = advantage_memories[player_id];

    if (memory->size() < memory->capacity()) {
        memory->add(info_state, advantages, weight);
    } else {
        float prob = static_cast<float>(memory->capacity()) / (memory->total_seen() + 1);
        if (std::uniform_real_distribution<>(0, 1)(rng) < prob) {
            // Replace random element
            int idx = std::uniform_int_distribution<>(0, memory->size() - 1)(rng);
            memory->replace(idx, info_state, advantages, weight);
        }
    }
}

void DeepCFR::initializeWeights(int num_iterations) {
    iteration_weights.resize(num_iterations);
    for (int t = 0; t < num_iterations; t++) {
        iteration_weights[t] = std::pow(t + 1, alpha); // As per paper
    }
}

float DeepCFR::computeIterationWeight(int t) {
    return iteration_weights[t];
}