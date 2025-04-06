#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"

typedef struct {
    size_t num_players_;
    size_t num_pots_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    Deck deck_;
    HandPhase hand_phase_;
} GameState;