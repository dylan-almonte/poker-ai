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

class Action {
private:
    ActionType action_type_;
    int player_id_;
    int amount_;

public:
    Action(ActionType action_type, int player_id, int amount) : 
        action_type_(action_type), 
        player_id_(player_id), 
        amount_(amount) {}
    ActionType getActionType() const { return action_type_; }
    int getPlayerId() const { return player_id_; }
    int getAmount() const { return amount_; }   
};
