#pragma once
#include <string>

enum class PlayerState {
    SKIP,    // Player sitting out this hand
    OUT,     // Player has folded their hand
    IN,      // Player is in latest pot with enough chips
    TO_CALL, // Player needs to call a raise
    ALL_IN   // Player is all-in, no more actions possible
};

std::string playerStateToString(PlayerState state); 