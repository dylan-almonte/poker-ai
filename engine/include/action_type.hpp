#pragma once
#include <string>

enum class ActionType {
    RAISE,  // Raises the latest pot
    ALL_IN, // Posts all available chips to the current pot
    CALL,   // Posts the minimum chips to remain in the pot
    CHECK,  // Passes the action
    FOLD    // Folds the hand and exits the pot
};

std::string actionTypeToString(ActionType type); 