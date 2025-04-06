#include "event_manager.hpp"

void EventManager::registerCallback(GameEventType type, EventCallback callback) {
    callbacks_[type].push_back(callback);
}

void EventManager::registerHandStartCallback(HandStartCallback callback) {
    registerCallback(GameEventType::HAND_START, 
        [callback](const GameEvent& e) {
            callback(static_cast<const HandStartEvent&>(e));
        });
}

void EventManager::registerHandEndCallback(HandEndCallback callback) {
    registerCallback(GameEventType::HAND_END,
        [callback](const GameEvent& e) {
            callback(static_cast<const HandEndEvent&>(e));
        });
}

void EventManager::registerPhaseChangeCallback(PhaseChangeCallback callback) {
    registerCallback(GameEventType::PHASE_CHANGE,
        [callback](const GameEvent& e) {
            callback(static_cast<const PhaseChangeEvent&>(e));
        });
}

void EventManager::registerPlayerActionCallback(PlayerActionCallback callback) {
    registerCallback(GameEventType::PLAYER_ACTION,
        [callback](const GameEvent& e) {
            callback(static_cast<const PlayerActionEvent&>(e));
        });
}

void EventManager::registerPlayerTurnCallback(PlayerTurnCallback callback) {
    registerCallback(GameEventType::PLAYER_TURN,
        [callback](const GameEvent& e) {
            callback(static_cast<const PlayerTurnEvent&>(e));
        });
}

void EventManager::dispatchEvent(const GameEvent& event) const {
    auto it = callbacks_.find(event.type);
    if (it != callbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

void EventManager::onHandStart(const Game* game, int button_pos) const {
    HandStartEvent event{GameEventType::HAND_START, game, button_pos};
    dispatchEvent(event);
}

void EventManager::onHandEnd(const Game* game, const std::vector<int>& winners, const std::vector<int>& payouts) const {
    HandEndEvent event{GameEventType::HAND_END, game, winners, payouts};
    dispatchEvent(event);
}

void EventManager::onPhaseChange(const Game* game, HandPhase::Phase old_phase, HandPhase::Phase new_phase, const std::vector<Card>& new_cards) const {
    PhaseChangeEvent event{GameEventType::PHASE_CHANGE, game, old_phase, new_phase, new_cards};
    dispatchEvent(event);
}

void EventManager::onPlayerAction(const Game* game, int player_id, const Action& action) const {
    PlayerActionEvent event{GameEventType::PLAYER_ACTION, game, player_id, action};
    dispatchEvent(event);
}

void EventManager::onPlayerTurn(const Game* game, int player_id, const std::vector<Action>& valid_actions) const {
    PlayerTurnEvent event{GameEventType::PLAYER_TURN, game, player_id, valid_actions};
    dispatchEvent(event);
} 