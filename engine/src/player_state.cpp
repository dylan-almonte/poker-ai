#include "player_state.hpp"
#include <string>

std::string playerStateToString(PlayerState state) {
    switch (state) {
        case PlayerState::SKIP: return "SKIP";
        case PlayerState::OUT: return "OUT";
        case PlayerState::IN: return "IN";
        case PlayerState::TO_CALL: return "TO_CALL";
        case PlayerState::ALL_IN: return "ALL_IN";
        default: return "UNKNOWN";
    }
} 