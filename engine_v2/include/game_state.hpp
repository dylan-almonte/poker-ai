#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "player_state.hpp"
#include <list>
#include <iostream>

struct GameState {
    bool is_terminal = false;
    size_t num_players{};
    size_t num_pots{};
    size_t current_player{};
    HandPhase::Phase street{};
    std::vector<PlayerState> player_states{};
    std::vector<int> player_chips{};
    std::vector<int> player_bets{};
    std::vector<int> player_rewards{};
    std::vector<Pot> pots{};
    std::vector<Card> board{5, Card(-1)};
    std::pair<Card, Card> hole_cards{Card(-1), Card(-1)};
    HandPhase::Phase hand_phase{HandPhase::Phase::PREFLOP};
    

    void print() const {
        std::cout << "GameState:" << std::endl;
        std::cout << "  is_terminal: " << is_terminal << std::endl;
        std::cout << "  num_players: " << num_players << std::endl;
        std::cout << "  num_pots: " << num_pots << std::endl;
        std::cout << "  street: " << phaseToString(street) << std::endl;
        std::cout << std::endl; 
        std::cout << "  current_player: " << current_player << std::endl;
        std::cout << "  hole_cards: " << hole_cards.first.toString() << " " << hole_cards.second.toString() << std::endl; 
        std::cout << "  board: ";
        for (const auto& card : board) {
            std::cout << card.toString() << " ";
        }
        std::cout << std::endl; 
        std::cout << std::endl; 

        std::cout << "  player_chips: ";
        for (const auto& chip : player_chips) {
            std::cout << chip << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "  players: " << std::endl;
        for (size_t i = 0; i < player_bets.size(); i++) {
            std::cout << "    player " << i << ": " << std::endl;
            std::cout << "\tchips: " << player_chips[i] << std::endl;
            std::cout << "\tbets: " << player_bets[i] << std::endl;
            std::cout << "\tstate: " << playerStateToString(player_states[i]) << std::endl;
            std::cout << "\treward: " << player_rewards[i] << std::endl;
        }
      
        std::cout << std::endl;
        std::cout << "  pots: ";
        for (const auto& pot : pots) {
            std::cout << pot.get_total_amount() << " ";
        }
        std::cout << std::endl;
        
    }
};