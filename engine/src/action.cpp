#include "action.hpp"
#include <string>

std::string actionTypeToString(ActionType type) {
    switch (type) {
        case ActionType::RAISE: return "RAISE";
        case ActionType::ALL_IN: return "ALL_IN";
        case ActionType::CALL: return "CALL";
        case ActionType::CHECK: return "CHECK";
        case ActionType::FOLD: return "FOLD";
        default: return "UNKNOWN";
    }
} 