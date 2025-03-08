// Deep Counterfactual Regret Minimization (Deep CFR) Complete Implementation
// Based on the paper by Noam Brown et al.

#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <memory>
#include <string>
#include <cmath>
#include <chrono>
#include <iomanip>

// Forward declarations
class Game;
class State;
class InfoState;
class NeuralNetwork;
class AdvantageNetwork;
class ValueNetwork;
class MLPAdvantageNetwork;
class MLPValueNetwork;

// Constants for Deep CFR
const int NUM_TRAVERSALS = 1000;  // Number of traversals per iteration
const int NUM_ITERATIONS = 100;   // Number of CFR iterations
const float LEARNING_RATE = 0.001f;
const int BATCH_SIZE = 128;

// Information state class (game-specific)
class InfoState {
public:
    InfoState(const std::string& key, const std::vector<float>& features) 
        : key_(key), features_(features) {}
    
    const std::string& key() const { return key_; }
    const std::vector<float>& features() const { return features_; }
    
private:
    std::string key_;
    std::vector<float> features_;
};

// State class (game-specific)
class State {
public:
    virtual ~State() = default;
    virtual bool is_terminal() const = 0;
    virtual float terminal_utility(int player) const = 0;
    virtual int current_player() const = 0;
    virtual std::vector<int> legal_actions() const = 0;
    virtual std::unique_ptr<State> clone() const = 0;
    virtual void apply_action(int action) = 0;
    virtual InfoState info_state() const = 0;
    virtual int num_players() const = 0;
};

// Game interface (implement for specific games)
class Game {
public:
    virtual ~Game() = default;
    virtual std::unique_ptr<State> new_initial_state() const = 0;
    virtual int num_players() const = 0;
    virtual int num_distinct_actions() const = 0;
};

// Neural network interface
class NeuralNetwork {
public:
    virtual ~NeuralNetwork() = default;
    virtual void train(const std::vector<std::vector<float>>& inputs, 
                      const std::vector<std::vector<float>>& targets) = 0;
    virtual std::vector<float> predict(const std::vector<float>& input) = 0;
};

// Advantage network interface
class AdvantageNetwork : public NeuralNetwork {
public:
    AdvantageNetwork(int num_actions) : num_actions_(num_actions) {}
    int num_actions() const { return num_actions_; }
    
private:
    int num_actions_;
};

// Value network interface
class ValueNetwork : public NeuralNetwork {
public:
    virtual ~ValueNetwork() = default;
};

// Memory buffer for advantage network training
struct AdvantageMemory {
    InfoState info_state;
    int action;
    float advantage;
    float weight;
    
    AdvantageMemory(const InfoState& is, int a, float adv, float w) 
        : info_state(is), action(a), advantage(adv), weight(w) {}
};

// Memory buffer for strategy network training
struct StrategyMemory {
    InfoState info_state;
    std::vector<float> strategy;
    float weight;
    
    StrategyMemory(const InfoState& is, const std::vector<float>& strat, float w) 
        : info_state(is), strategy(strat), weight(w) {}
};

// Import neural network implementation
// #include "neural_network.h"

// Deep CFR implementation
class DeepCFR {
public:
    DeepCFR(int num_players, int num_actions, int input_size) 
        : num_players_(num_players), num_actions_(num_actions), input_size_(input_size) {
        // Initialize advantage networks for each player
        for (int p = 0; p < num_players; ++p) {
            advantage_networks_.push_back(std::make_unique<MLPAdvantageNetwork>(input_size, num_actions));
        }
        
        // Initialize strategy network
        strategy_network_ = std::make_unique<MLPValueNetwork>(input_size, num_actions);
        
        // Setup RNG
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
    
    // Main Deep CFR algorithm
    void train(const Game& game) {
        std::vector<std::vector<AdvantageMemory>> advantage_memories(num_players_);
        std::vector<StrategyMemory> strategy_memories;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Main loop of Deep CFR
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
            std::cout << "Starting iteration " << iter + 1 << "/" << NUM_ITERATIONS << std::endl;
            
            // External sampling CFR
            for (int p = 0; p < num_players_; ++p) {
                std::cout << "  Training player " << p << " advantage network..." << std::endl;
                for (int t = 0; t < NUM_TRAVERSALS; ++t) {
                    std::unique_ptr<State> root = game.new_initial_state();
                    traverse(*root, p, 1.0, advantage_memories[p]);
                }
                
                // Train advantage network for player p
                if (advantage_memories[p].size() >= BATCH_SIZE) {
                    train_advantage_network(p, advantage_memories[p]);
                }
            }
            
            // Collect strategy data using current approximate CFR strategy
            std::cout << "  Collecting strategy data..." << std::endl;
            collect_strategy_data(game, strategy_memories);
            
            // Train strategy network
            if (strategy_memories.size() >= BATCH_SIZE) {
                std::cout << "  Training strategy network..." << std::endl;
                train_strategy_network(strategy_memories);
            }
            
            // Log progress
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
            std::cout << "  Time elapsed: " << elapsed << "s" << std::endl;
        }
    }
    
    // Get a strategy for a given information state
    std::vector<float> get_strategy(const InfoState& info_state) {
        return strategy_network_->predict(info_state.features());
    }
    
private:
    int num_players_;
    int num_actions_;
    int input_size_;
    std::vector<std::unique_ptr<AdvantageNetwork>> advantage_networks_;
    std::unique_ptr<ValueNetwork> strategy_network_;
    std::mt19937 rng_;
    
    // Traverse game tree using external sampling MCCFR
    float traverse(const State& state, int traversing_player, float reach_prob, 
                   std::vector<AdvantageMemory>& advantage_memories) {
        if (state.is_terminal()) {
            return state.terminal_utility(traversing_player);
        }
        
        int current_player = state.current_player();
        
        // Chance node
        if (current_player < 0) {
            // Sample chance outcome
            std::vector<int> legal_actions = state.legal_actions();
            std::uniform_int_distribution<int> dist(0, legal_actions.size() - 1);
            int action_idx = dist(rng_);
            int action = legal_actions[action_idx];
            
            std::unique_ptr<State> next_state = state.clone();
            next_state->apply_action(action);
            
            return traverse(*next_state, traversing_player, reach_prob, advantage_memories);
        }
        
        const InfoState& info_state = state.info_state();
        std::vector<int> legal_actions = state.legal_actions();
        
        // Current player is the traversing player
        if (current_player == traversing_player) {
            // Get advantages for all actions using current network
            std::vector<float> advantages = advantage_networks_[current_player]->predict(info_state.features());
            
            // Convert advantages to strategy using regret matching
            std::vector<float> strategy = regret_matching(advantages, legal_actions);
            
            // Sample an action from the strategy
            int action = sample_action_from_strategy(strategy, legal_actions);
            
            // Recursively traverse
            std::unique_ptr<State> next_state = state.clone();
            next_state->apply_action(action);
            float counterfactual_value = traverse(*next_state, traversing_player, reach_prob, advantage_memories);
            
            // Compute advantages for all actions
            std::vector<float> action_values(num_actions_, 0.0f);
            for (int a : legal_actions) {
                std::unique_ptr<State> next_state_a = state.clone();
                next_state_a->apply_action(a);
                action_values[a] = traverse(*next_state_a, traversing_player, 0.0, advantage_memories);
            }
            
            // Compute regrets and add to memory buffer
            for (int a : legal_actions) {
                float regret = action_values[a] - counterfactual_value;
                advantage_memories.emplace_back(info_state, a, regret, reach_prob);
            }
            
            return counterfactual_value;
        }
        // Current player is the opponent
        else {
            // Get strategy for opponent
            std::vector<float> advantages = advantage_networks_[current_player]->predict(info_state.features());
            std::vector<float> strategy = regret_matching(advantages, legal_actions);
            
            // Sample an action for the opponent
            int action = sample_action_from_strategy(strategy, legal_actions);
            
            // Recursively traverse
            std::unique_ptr<State> next_state = state.clone();
            next_state->apply_action(action);
            return traverse(*next_state, traversing_player, reach_prob * strategy[action], advantage_memories);
        }
    }
    
    // Convert regrets to a strategy using regret matching
    std::vector<float> regret_matching(const std::vector<float>& advantages, const std::vector<int>& legal_actions) {
        std::vector<float> strategy(num_actions_, 0.0f);
        float sum_positive_regrets = 0.0f;
        
        // Sum positive regrets for legal actions
        for (int a : legal_actions) {
            if (advantages[a] > 0) {
                sum_positive_regrets += advantages[a];
            }
        }
        
        // Normalize with positive regrets
        if (sum_positive_regrets > 0) {
            for (int a : legal_actions) {
                strategy[a] = advantages[a] > 0 ? advantages[a] / sum_positive_regrets : 0.0f;
            }
        } else {
            // Uniform strategy if all regrets are non-positive
            float uniform_prob = 1.0f / legal_actions.size();
            for (int a : legal_actions) {
                strategy[a] = uniform_prob;
            }
        }
        
        return strategy;
    }
    
    // Sample an action based on a strategy probability distribution
    int sample_action_from_strategy(const std::vector<float>& strategy, const std::vector<int>& legal_actions) {
        std::vector<float> probs;
        for (int a : legal_actions) {
            probs.push_back(strategy[a]);
        }
        
        std::discrete_distribution<int> dist(probs.begin(), probs.end());
        int idx = dist(rng_);
        return legal_actions[idx];
    }
    
    // Train advantage network for a specific player
    void train_advantage_network(int player, std::vector<AdvantageMemory>& memories) {
        // Prepare training data
        std::vector<std::vector<float>> inputs;
        std::vector<std::vector<float>> targets;
        
        for (const auto& memory : memories) {
            inputs.push_back(memory.info_state.features());
            std::vector<float> target(num_actions_, 0.0f);
            target[memory.action] = memory.advantage;
            targets.push_back(target);
        }
        
        // Train the network
        advantage_networks_[player]->train(inputs, targets);
        
        // Clear memory buffer
        memories.clear();
    }
    
    // Collect strategy data for training the strategy network
    void collect_strategy_data(const Game& game, std::vector<StrategyMemory>& strategy_memories) {
        std::unique_ptr<State> root = game.new_initial_state();
        collect_strategy_recursive(*root, 1.0, strategy_memories);
    }
    
    // Recursively collect strategy data
    void collect_strategy_recursive(const State& state, float reach_prob, 
                                   std::vector<StrategyMemory>& strategy_memories) {
        if (state.is_terminal()) {
            return;
        }
        
        int current_player = state.current_player();
        
        // Chance node
        if (current_player < 0) {
            // Sample chance outcome
            std::vector<int> legal_actions = state.legal_actions();
            std::uniform_int_distribution<int> dist(0, legal_actions.size() - 1);
            int action_idx = dist(rng_);
            int action = legal_actions[action_idx];
            
            std::unique_ptr<State> next_state = state.clone();
            next_state->apply_action(action);
            
            collect_strategy_recursive(*next_state, reach_prob, strategy_memories);
        }
        else {
            const InfoState& info_state = state.info_state();
            std::vector<int> legal_actions = state.legal_actions();
            
            // Get strategy for this player
            std::vector<float> advantages = advantage_networks_[current_player]->predict(info_state.features());
            std::vector<float> strategy = regret_matching(advantages, legal_actions);
            
            // Add to strategy memories
            strategy_memories.emplace_back(info_state, strategy, reach_prob);
            
            // Sample an action and continue
            int action = sample_action_from_strategy(strategy, legal_actions);
            std::unique_ptr<State> next_state = state.clone();
            next_state->apply_action(action);
            
            collect_strategy_recursive(*next_state, reach_prob * strategy[action], strategy_memories);
        }
    }
    
    // Train strategy network
    void train_strategy_network(std::vector<StrategyMemory>& memories) {
        // Prepare training data
        std::vector<std::vector<float>> inputs;
        std::vector<std::vector<float>> targets;
        
        for (const auto& memory : memories) {
            inputs.push_back(memory.info_state.features());
            targets.push_back(memory.strategy);
        }
        
        // Train the network
        strategy_network_->train(inputs, targets);
        
        // Clear memory buffer
        memories.clear();
    }
};

// Import Kuhn Poker game implementation
// #include "kuhn_poker.h"

// Main function
int main() {
    std::cout << "Deep CFR Implementation for Kuhn Poker" << std::endl;
    std::cout << "Based on the paper by Noam Brown et al." << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Create the game (Kuhn Poker)
    KuhnPoker game;
    
    // Initialize Deep CFR (9 input features for Kuhn Poker's info state)
    DeepCFR deepCFR(game.num_players(), game.num_distinct_actions(), 9);
    
    // Train the agent
    std::cout << "Starting training..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    deepCFR.train(game);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    std::cout << "Training completed in " << duration << " seconds." << std::endl;
    
    // Test the trained strategy
    std::cout << "Testing strategy for various information states:" << std::endl;
    
    // Player 0 with different cards and histories
    std::vector<std::pair<std::string, std::vector<float>>> test_cases = {
        {"Player 0 with Jack (0), no history", {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}},
        {"Player 0 with Queen (1), no history", {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}},
        {"Player 0 with King (2), no history", {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}},
        {"Player 0 with Jack (0), opponent passed", {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}},
        {"Player 0 with King (2), opponent bet", {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}},
    };
    
    for (const auto& test_case : test_cases) {
        InfoState info_state("test", test_case.second);
        std::vector<float> strategy = deepCFR.get_strategy(info_state);
        
        std::cout << test_case.first << ":" << std::endl;
        std::cout << "  PASS: " << std::fixed << std::setprecision(4) << strategy[0] << std::endl;
        std::cout << "  BET: " << std::fixed << std::setprecision(4) << strategy[1] << std::endl;
    }
    
    return 0;
}