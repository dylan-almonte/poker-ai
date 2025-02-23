#include "actions.hpp"

namespace poker {

Action::Action(ActionType type, int amount) 
    : type_(type), amount_(amount) {}

ActionType Action::getType() const {
    return type_;
}

int Action::getAmount() const {
    return amount_;
}

bool Action::isValid() const {
    switch (type_) {
        case ActionType::FOLD:
            return amount_ == 0;
        case ActionType::CHECK:
            return amount_ == 0;
        case ActionType::CALL:
            return amount_ >= 0;
        case ActionType::BET:
        case ActionType::RAISE:
            return amount_ > 0;
        default:
            return false;
    }
}

std::string Action::toString() const {
    switch (type_) {
        case ActionType::FOLD:  return "FOLD";
        case ActionType::CHECK: return "CHECK";
        case ActionType::CALL:  return "CALL " + std::to_string(amount_);
        case ActionType::BET:   return "BET " + std::to_string(amount_);
        case ActionType::RAISE: return "RAISE " + std::to_string(amount_);
        default:               return "INVALID";
    }
}

} // namespace poker 