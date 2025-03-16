#pragma once
#include <vector>
#include <memory>
#include <deque>
#include <coroutine>
#include <functional>
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"

// Coroutine support structures
struct ActionAwaiter {
    ActionAwaiter() : handle(nullptr), action(ActionType::FOLD, 0) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) noexcept { handle = h; }
    Action await_resume() const noexcept { return action; }
    std::coroutine_handle<> handle;
    Action action;
};

struct BettingRoundPromise;

struct BettingRoundCoroutine {
    struct promise_type {
        BettingRoundCoroutine get_return_object() { 
            return BettingRoundCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)}; 
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    BettingRoundCoroutine(std::coroutine_handle<promise_type> h) : handle(h) {}
    std::coroutine_handle<promise_type> handle;
};

class HandPhaseStateMachine {
public:
    using TransitionCallback = std::function<void(HandPhase::Phase)>;
    
    HandPhaseStateMachine() : current_phase_(HandPhase::Phase::PREHAND) {}
    
    void transition(bool round_complete, bool hand_over) {
        if (hand_over) {
            setPhase(HandPhase::Phase::SETTLE);
            return;
        }
        
        if (round_complete) {
            switch (current_phase_) {
                case HandPhase::Phase::PREHAND:
                    setPhase(HandPhase::Phase::PREFLOP);
                    break;
                case HandPhase::Phase::PREFLOP:
                    setPhase(HandPhase::Phase::FLOP);
                    break;
                case HandPhase::Phase::FLOP:
                    setPhase(HandPhase::Phase::TURN);
                    break;
                case HandPhase::Phase::TURN:
                    setPhase(HandPhase::Phase::RIVER);
                    break;
                case HandPhase::Phase::RIVER:
                    setPhase(HandPhase::Phase::SETTLE);
                    break;
                default:
                    break;
            }
        }
    }
    
    HandPhase::Phase getCurrentPhase() const { return current_phase_; }
    void setPhase(HandPhase::Phase phase) { 
        current_phase_ = phase;
        if (on_transition_) {
            on_transition_(phase);
        }
    }
    
    void setTransitionCallback(TransitionCallback callback) {
        on_transition_ = std::move(callback);
    }

private:
    HandPhase::Phase current_phase_;
    TransitionCallback on_transition_;
};

class Game {
private:
    std::vector<Action> action_history_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    std::vector<Card> board_;
    Deck deck_;
    
    int btn_loc_;
    int sb_loc_;
    int bb_loc_;
    int current_player_;
    HandPhaseStateMachine phase_machine_;

    int small_blind_;
    int big_blind_;
    int last_raise_ = 0;
    bool raise_option_ = true;
    bool _isHandOver() const;
    void _takeAction(Action action);

    // Coroutine state
    std::coroutine_handle<> current_betting_round_;
    ActionAwaiter action_awaiter_;

    void _dealCards();
    void _postPlayerBets(int player_idx, int amount);
    void _splitPot(int pot_idx, int raise_level);
    void _moveBlinds();
    void _settleHand();
    void _handleAction(Action action);
    bool _isValidAction(Action action) const;
    Action _translateAllIn(Action action);
    BettingRoundCoroutine _bettingRound(HandPhase::Phase phase);
    std::vector<int> _player_iter(
        std::optional<int> loc = std::nullopt,
        bool reverse = false,
        std::vector<PlayerState> match_states = {},
        std::vector<PlayerState> filter_states = {});

public:
    Game(int num_players, int starting_chips, int small_blind, int big_blind);
    ~Game() {
        if (current_betting_round_) current_betting_round_.destroy();
    }
    
    // Core game flow methods
    void startHand(int btn_loc = -1);
    void takeAction(Action action);
    bool isHandOver() const;
    
    // Getters
    const std::vector<Card>& getBoard() const { return board_; }
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    int getCurrentPlayer() const { return current_player_; }
    HandPhase::Phase getPhase() const { return phase_machine_.getCurrentPhase(); }
    const std::vector<Action>& getActionHistory() const { return action_history_; }
    
    // Player iteration helpers
    std::vector<int> getActivePlayers(int loc = 0) const;
    int getNextActivePlayer(int from) const;
    
    // Game state helpers
    float getPayoff(int player_idx) const;
    int getInitialStackTotal() const;
    int getTotalToCall(int player_id) const;
    
    // Debug helper
    void printState() const;
}; 