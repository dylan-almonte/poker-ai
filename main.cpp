#include "game.hpp"
#include "player.hpp"
#include "card.hpp"
#include <memory>
#include <iostream>

class MyPlayer : public Player {
public:
    MyPlayer(int id, const std::string& name, int chips) 
        : Player(id, name, chips) {}
    
    // Implement your player logic here
    void takeAction(ActionType& action, int& amount) {
        // Simple example: always call
        action = ActionType::CALL;
        amount = 0;  // Amount will be set by the game engine for calls
    }
};

int main() {
    try {
        // Create a game with 6 players, 1000 starting chips, 10/20 blinds
        Game game(6, 1000, 10, 20);
        
        // Add your players
        for (int i = 0; i < 6; i++) {
            auto player = std::make_shared<MyPlayer>(
                i,  // Add player ID
                "Player " + std::to_string(i), 
                1000
            );
            // Instead of addPlayer, just construct with correct number of players
        }

        // Play some hands
        for (int i = 0; i < 10; i++) {
            std::cout << "\nPlaying hand " << i + 1 << std::endl;
            
            game.startHand();
            
            while (!game.isHandComplete()) {
                // Instead of processNextAction, use takeAction
                ActionType action = game.getPhase() == HandPhase::PREFLOP ? ActionType::CALL : ActionType::CHECK;
                int amount = 0;
                game.takeAction(action, amount);
                
                // You can get game state
                const auto& board = game.getBoard();
                const auto& players = game.getPlayers();
                const auto& pots = game.getPots();
                
                // Print current state if desired
                std::cout << "Board: ";
                for (const auto& card : board) {
                    std::cout << card.toString() << " ";
                }
                std::cout << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 