/** 
 * @file action.hpp
 * @brief Defines the Action class and related enums
 * 
 * This file contains the definition of the Action class, which represents a player's action in a poker game.
 * It includes the ActionType enum, which specifies the type of action (raise, all in, call, check, fold),
 * and the Action class, which represents an action with an optional amount and player ID.
 */

#pragma once
#include <string>
enum class ActionType {
    RAISE,  // Raises the latest pot
    ALL_IN, // Posts all available chips to the current pot
    CALL,   // Posts the minimum chips to remain in the pot
    CHECK,  // Passes the action
    FOLD    // Folds the hand and exits the pot
};

class Action {
private:
    ActionType action_type_;
    int amount_;
    int player_id_;
public:
    Action(ActionType action_type) : 
        action_type_(action_type), 
        amount_(0),
        player_id_(-1) {}
    Action(ActionType action_type, int amount) : 
        action_type_(action_type), 
        amount_(amount),
        player_id_(-1) {}
    Action(ActionType action_type, int player_id, int amount) : 
        action_type_(action_type), 
        amount_(amount),
        player_id_(player_id) {}
    ActionType getActionType() const { return action_type_; }
    int getPlayerId() const { return player_id_; }
    int getAmount() const { return amount_; }   
};

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