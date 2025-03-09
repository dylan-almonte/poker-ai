#include "engine.hpp"
#include "deep_cfr.hpp"
#include "deep_cfr_player.hpp"
#include <memory>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

// Helper function to format time
std::string formatDuration(std::chrono::seconds duration) {
    int hours = duration.count() / 3600;
    int minutes = (duration.count() % 3600) / 60;
    int seconds = duration.count() % 60;
    
    std::stringstream ss;
    if (hours > 0) ss << hours << "h ";
    if (minutes > 0 || hours > 0) ss << minutes << "m ";
    ss << seconds << "s";
    return ss.str();
}

// Helper function to print a separator line
void printSeparator() {
    std::cout << "=========================================================" << std::endl;
}

int main(int argc, char** argv) {
    try {
        printSeparator();
        std::cout << "Deep CFR Poker AI - Starting up" << std::endl;
        printSeparator();
        
        // Parse command line arguments
        bool train_mode = false;
        int num_iterations = 100;
        int num_traversals = 10000;
        int num_players = 6;
        int starting_chips = 1000;
        int small_blind = 10;
        int big_blind = 20;
        int num_hands = 10;
        std::string model_path = "models/latest";
        
        std::cout << "Parsing command line arguments..." << std::endl;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--train") {
                train_mode = true;
                std::cout << "  Training mode enabled" << std::endl;
            } else if (arg == "--iterations" && i + 1 < argc) {
                num_iterations = std::stoi(argv[++i]);
                std::cout << "  Number of iterations: " << num_iterations << std::endl;
            } else if (arg == "--traversals" && i + 1 < argc) {
                num_traversals = std::stoi(argv[++i]);
                std::cout << "  Number of traversals per iteration: " << num_traversals << std::endl;
            } else if (arg == "--players" && i + 1 < argc) {
                num_players = std::stoi(argv[++i]);
                std::cout << "  Number of players: " << num_players << std::endl;
            } else if (arg == "--chips" && i + 1 < argc) {
                starting_chips = std::stoi(argv[++i]);
                std::cout << "  Starting chips: " << starting_chips << std::endl;
            } else if (arg == "--small-blind" && i + 1 < argc) {
                small_blind = std::stoi(argv[++i]);
                std::cout << "  Small blind: " << small_blind << std::endl;
            } else if (arg == "--big-blind" && i + 1 < argc) {
                big_blind = std::stoi(argv[++i]);
                std::cout << "  Big blind: " << big_blind << std::endl;
            } else if (arg == "--hands" && i + 1 < argc) {
                num_hands = std::stoi(argv[++i]);
                std::cout << "  Number of hands to play: " << num_hands << std::endl;
            } else if (arg == "--model" && i + 1 < argc) {
                model_path = argv[++i];
                std::cout << "  Model path: " << model_path << std::endl;
            } else {
                std::cout << "  Unknown argument: " << arg << std::endl;
            }
        }
        
        printSeparator();
        std::cout << "Creating Deep CFR agent..." << std::endl;
        
        // Create Deep CFR agent
        auto deep_cfr = std::make_shared<DeepCFR>(num_players, num_traversals);
        
        // Train or load the model
        if (train_mode) {
            printSeparator();
            std::cout << "Training Deep CFR for " << num_iterations << " iterations..." << std::endl;
            printSeparator();
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Train with progress updates
            for (int iter = 0; iter < num_iterations; iter++) {
                auto iter_start = std::chrono::high_resolution_clock::now();
                
                std::cout << "Iteration " << iter + 1 << "/" << num_iterations << std::endl;
                
                // Perform one iteration of training
                deep_cfr->train(1, 128, 128);
                
                auto iter_end = std::chrono::high_resolution_clock::now();
                auto iter_duration = std::chrono::duration_cast<std::chrono::seconds>(iter_end - iter_start);
                
                // Calculate ETA
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(iter_end - start_time);
                auto avg_iter_time = elapsed.count() / (iter + 1);
                auto remaining = std::chrono::seconds(avg_iter_time * (num_iterations - iter - 1));
                
                std::cout << "  Iteration completed in " << formatDuration(iter_duration) << std::endl;
                std::cout << "  Elapsed time: " << formatDuration(elapsed) << std::endl;
                std::cout << "  Estimated time remaining: " << formatDuration(remaining) << std::endl;
                printSeparator();
                
                // Save model periodically
                if ((iter + 1) % 10 == 0 || iter == num_iterations - 1) {
                    std::cout << "Saving model checkpoint to models/iter_" << (iter + 1) << std::endl;
                    deep_cfr->saveModels("models/iter_" + std::to_string(iter + 1));
                }
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
            
            printSeparator();
            std::cout << "Training completed in " << formatDuration(duration) << std::endl;
            
            // Save the trained model
            std::cout << "Saving final model to " << model_path << std::endl;
            deep_cfr->saveModels(model_path);
            printSeparator();
        } else {
            // Load the model
            printSeparator();
            std::cout << "Loading model from " << model_path << std::endl;
            deep_cfr->loadModels(model_path);
            printSeparator();
        }
        
        // Create a game with the specified number of players and settings
        std::cout << "Creating poker game with " << num_players << " players" << std::endl;
        std::cout << "  Starting chips: " << starting_chips << std::endl;
        std::cout << "  Blinds: " << small_blind << "/" << big_blind << std::endl;
        
        
        // Add Deep CFR players
        std::vector<std::shared_ptr<DeepCFRPlayer>> cfr_players;
        std::cout << "Adding players to the game..." << std::endl;
        for (int i = 0; i < num_players; i++) {
            auto player = std::make_shared<DeepCFRPlayer>(
                i,  // Player ID
                "DeepCFR Player " + std::to_string(i), 
                starting_chips,  // Starting chips
                deep_cfr,
                true,  // Enable exploration
                0.05   // Exploration probability
            );
            cfr_players.push_back(player);
            std::cout << "  Added player " << i << ": " << player->getName() << std::endl;
        }
        
        printSeparator();
        std::cout << "Playing " << num_hands << " hands..." << std::endl;
        printSeparator();
        
        // Play some hands
        for (int hand = 0; hand < num_hands; hand++) {
            std::cout << "\n=== Hand " << hand + 1 << " ===" << std::endl;
            Game game(num_players, starting_chips, small_blind, big_blind);
            
            game.startHand();
            std::cout << "Hand started. Initial state:" << std::endl;
            game.printState();
            
            int action_count = 0;
            while (!game.isHandComplete()) {
                action_count++;
                int current_player_id = game.getCurrentPlayer();
                auto current_player = cfr_players[current_player_id];
                
                std::cout << "\nAction #" << action_count << ": Player " << current_player_id 
                          << " (" << current_player->getName() << ") to act" << std::endl;
                
                Action action = current_player->takeAction(game);
                
                // Get action from the current player
                std::cout << "  Getting action from player..." << std::endl;
                
                std::cout << "  Player " << current_player_id << " takes action: " 
                          << actionTypeToString(action.getActionType());
                
                if (action.getActionType() == ActionType::RAISE || action.getActionType() == ActionType::ALL_IN) {
                    std::cout << " " << action.getAmount();
                }
                std::cout << std::endl;
                
                // Apply the action
                game.takeAction(action);
                
                std::cout << "  Updated game state:" << std::endl;
                game.printState();
            }
            game.settleHand();
            // Print hand results
            std::cout << "\n=== Hand " << hand + 1 << " Results ===" << std::endl;
            for (const auto& player : game.getPlayers()) {
                std::cout << "  Player " << player->getId() << " (" << player->getName() 
                          << "): " << player->getChips() << " chips" << std::endl;
            }
            printSeparator();
        }
        
        // Print final results
        std::cout << "\n=== Final Results after " << num_hands << " hands ===" << std::endl;
        for (const auto& player : cfr_players) {
            int profit = player->getChips() - starting_chips;
            std::cout << "  Player " << player->getId() << " (" << player->getName() 
                      << "): " << player->getChips() << " chips (";
            
            if (profit > 0) std::cout << "+";
            std::cout << profit << ")" << std::endl;
        }
        printSeparator();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Program completed successfully." << std::endl;
    return 0;
} 