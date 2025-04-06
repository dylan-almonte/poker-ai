#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include "action.hpp"
#include "hand_phase.hpp"
#include "player.hpp"

// Forward declarations
class Game;

// Event types
enum class GameEventType {
    HAND_START,
    HAND_END,
    PHASE_CHANGE,
    PLAYER_ACTION,
    PLAYER_TURN,
    POT_UPDATE,
    BOARD_UPDATE
};

// Event data structures
struct GameEvent {
    GameEventType type;
    const Game* game;
};

struct HandStartEvent : GameEvent {
    int button_pos;
};

struct HandEndEvent : GameEvent {
    std::vector<int> winners;
    std::vector<int> payouts;
};

struct PhaseChangeEvent : GameEvent {
    HandPhase::Phase old_phase;
    HandPhase::Phase new_phase;
    std::vector<Card> new_cards;
};

struct PlayerActionEvent : GameEvent {
    int player_id;
    Action action;
};

struct PlayerTurnEvent : GameEvent {
    int player_id;
    std::vector<Action> valid_actions;
};

// Event callback types
using EventCallback = std::function<void(const GameEvent&)>;
using HandStartCallback = std::function<void(const HandStartEvent&)>;
using HandEndCallback = std::function<void(const HandEndEvent&)>;
using PhaseChangeCallback = std::function<void(const PhaseChangeEvent&)>;
using PlayerActionCallback = std::function<void(const PlayerActionEvent&)>;
using PlayerTurnCallback = std::function<void(const PlayerTurnEvent&)>;

class EventManager {
private:
    std::unordered_map<GameEventType, std::vector<EventCallback>> callbacks_;

public:
    // Registration methods
    void registerCallback(GameEventType type, EventCallback callback);
    void registerHandStartCallback(HandStartCallback callback);
    void registerHandEndCallback(HandEndCallback callback);
    void registerPhaseChangeCallback(PhaseChangeCallback callback);
    void registerPlayerActionCallback(PlayerActionCallback callback);
    void registerPlayerTurnCallback(PlayerTurnCallback callback);

    // Event dispatch methods
    void dispatchEvent(const GameEvent& event) const;
    void onHandStart(const Game* game, int button_pos) const;
    void onHandEnd(const Game* game, const std::vector<int>& winners, const std::vector<int>& payouts) const;
    void onPhaseChange(const Game* game, HandPhase::Phase old_phase, HandPhase::Phase new_phase, const std::vector<Card>& new_cards) const;
    void onPlayerAction(const Game* game, int player_id, const Action& action) const;
    void onPlayerTurn(const Game* game, int player_id, const std::vector<Action>& valid_actions) const;
};
