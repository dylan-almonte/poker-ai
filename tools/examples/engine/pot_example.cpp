#include "pot.hpp"
#include <iostream>
#include <iomanip>

/**
 * @brief Example function demonstrating how to use the Pot class
 */
void pot_example() {
    // Create a new pot
    Pot pot;
    
    // Players post blinds
    pot.player_post(0, 1);  // Small blind
    pot.player_post(1, 2);  // Big blind
    
    std::cout << "Initial pot state:" << std::endl;
    std::cout << "  Total amount: " << pot.get_total_amount() << std::endl;
    std::cout << "  Raised amount: " << pot.get_raised() << std::endl;
    
    // Player 2 calls the big blind
    pot.player_post(2, 2);
    
    // Player 3 raises to 6
    pot.player_post(3, 6);
    
    std::cout << "\nAfter betting:" << std::endl;
    std::cout << "  Total amount: " << pot.get_total_amount() << std::endl;
    std::cout << "  Raised amount: " << pot.get_raised() << std::endl;
    
    // Show how much each player has bet
    std::cout << "\nPlayer bets:" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << "  Player " << i << ": " << pot.get_player_amount(i) << std::endl;
    }
    
    // Show how much each player needs to call
    std::cout << "\nChips to call:" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << "  Player " << i << ": " << pot.chips_to_call(i) << std::endl;
    }
    
    // Player 0 calls the raise
    pot.player_post(0, pot.chips_to_call(0));
    
    // Player 1 calls the raise
    pot.player_post(1, pot.chips_to_call(1));
    
    // Player 2 calls the raise
    pot.player_post(2, pot.chips_to_call(2));
    
    std::cout << "\nAfter all players call:" << std::endl;
    std::cout << "  Total amount: " << pot.get_total_amount() << std::endl;
    std::cout << "  Raised amount: " << pot.get_raised() << std::endl;
    
    // Collect bets for the next round
    pot.collect_bets();
    
    std::cout << "\nAfter collecting bets:" << std::endl;
    std::cout << "  Total amount: " << pot.get_total_amount() << std::endl;
    std::cout << "  Raised amount: " << pot.get_raised() << std::endl;
    
    // Example of using the immutable version
    Pot new_pot = pot.with_player_post(0, 10);
    
    std::cout << "\nAfter immutable player post:" << std::endl;
    std::cout << "  Original pot total: " << pot.get_total_amount() << std::endl;
    std::cout << "  New pot total: " << new_pot.get_total_amount() << std::endl;
    
    // Example of splitting a pot
    pot.player_post(0, 20);
    pot.player_post(1, 30);
    pot.player_post(2, 10);
    
    std::cout << "\nBefore splitting pot:" << std::endl;
    std::cout << "  Total amount: " << pot.get_total_amount() << std::endl;
    std::cout << "  Raised amount: " << pot.get_raised() << std::endl;
    
    auto split_pot = pot.split_pot(15);
    
    if (split_pot) {
        std::cout << "\nAfter splitting pot:" << std::endl;
        std::cout << "  Main pot total: " << pot.get_total_amount() << std::endl;
        std::cout << "  Main pot raised: " << pot.get_raised() << std::endl;
        std::cout << "  Side pot total: " << split_pot->get_total_amount() << std::endl;
        std::cout << "  Side pot raised: " << split_pot->get_raised() << std::endl;
    }
}

int main() {
    pot_example();
    return 0;
} 