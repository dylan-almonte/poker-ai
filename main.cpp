#include "engine.hpp"
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
        game.startHand();
        game.printState(); 
        game.takeAction(ActionType::CALL);
        game.printState();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 