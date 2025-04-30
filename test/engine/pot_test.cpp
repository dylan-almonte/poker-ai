#include "pot.hpp"
#include <cassert>
#include <iostream>

/**
 * @brief Test function for the Pot class
 */
void test_pot() {
    // Test basic functionality
    Pot pot;
    assert(pot.get_amount() == 0);
    assert(pot.get_raised() == 0);
    assert(pot.get_total_amount() == 0);
    
    // Test player_post
    pot.player_post(1, 10);
    assert(pot.get_player_amount(1) == 10);
    assert(pot.get_raised() == 10);
    assert(pot.get_total_amount() == 10);
    
    // Test chips_to_call
    assert(pot.chips_to_call(1) == 0);
    assert(pot.chips_to_call(2) == 10);
    
    // Test player_post with multiple players
    pot.player_post(2, 20);
    assert(pot.get_player_amount(2) == 20);
    assert(pot.get_raised() == 20);
    assert(pot.get_total_amount() == 30);
    
    // Test chips_to_call after raise
    assert(pot.chips_to_call(1) == 10);
    assert(pot.chips_to_call(2) == 0);
    
    // Test collect_bets
    pot.collect_bets();
    assert(pot.get_amount() == 30);
    assert(pot.get_raised() == 0);
    assert(pot.get_total_amount() == 30);
    assert(pot.get_player_amount(1) == 0);
    assert(pot.get_player_amount(2) == 0);
    
    // Test remove_player
    pot.player_post(1, 10);
    pot.remove_player(1);
    assert(pot.get_amount() == 40);
    assert(pot.get_player_amount(1) == 0);
    
    // Test players_in_pot
    pot.player_post(1, 10);
    pot.player_post(2, 10);
    pot.player_post(3, 10);
    
    auto players = pot.players_in_pot();
    assert(players.size() == 3);
    assert(std::find(players.begin(), players.end(), 1) != players.end());
    assert(std::find(players.begin(), players.end(), 2) != players.end());
    assert(std::find(players.begin(), players.end(), 3) != players.end());
    
    // Test with_player_post (immutable version)
    Pot new_pot = pot.with_player_post(4, 20);
    assert(pot.get_player_amount(4) == 0);
    assert(new_pot.get_player_amount(4) == 20);
    assert(new_pot.get_raised() == 20);
    
    // Test split_pot
    pot.player_post(1, 30);
    pot.player_post(2, 20);
    pot.player_post(3, 10);
    
    auto split_pot = pot.split_pot(15);
    assert(split_pot != nullptr);
    assert(pot.get_raised() == 15);
    assert(split_pot->get_raised() == 15);
    assert(pot.get_player_amount(1) == 15);
    assert(split_pot->get_player_amount(1) == 15);
    assert(pot.get_player_amount(2) == 15);
    assert(split_pot->get_player_amount(2) == 5);
    assert(pot.get_player_amount(3) == 10);
    assert(split_pot->get_player_amount(3) == 0);
    
    std::cout << "All Pot tests passed!" << std::endl;
}

int main() {
    test_pot();
    return 0;
} 