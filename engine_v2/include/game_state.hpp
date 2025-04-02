#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"

template <size_t N>
class GameState {
    public:
        std::vector<std::shared_ptr<Player>> players_;
        
};

