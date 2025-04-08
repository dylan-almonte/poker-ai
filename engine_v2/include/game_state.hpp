#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "player_state.hpp"
#include <list>

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
    
};