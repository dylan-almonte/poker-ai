#pragma once
#include <memory>
#include <vector>
#include "player.hpp"
#include "pot.hpp"
#include "action.hpp"

class BettingStateMachine;

// Base class for all betting states
class BettingState {
public:
    virtual ~BettingState() = default;
    
    // Called when entering this state
    virtual void enter(BettingStateMachine& machine) = 0;
    
    // Called when exiting this state
    virtual void exit(BettingStateMachine& machine) = 0;
    
    // Handle an action in this state
    virtual void handleAction(BettingStateMachine& machine, const Action& action) = 0;
    
    // Get the name of this state
    virtual std::string getName() const = 0;
};

// State machine that manages betting round states
class BettingStateMachine {
private:
    std::unique_ptr<BettingState> current_state_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    int current_player_;
    int last_to_act_;
    int all_in_count_;
    bool round_complete_;
    
public:
    BettingStateMachine(std::vector<std::shared_ptr<Player>> players,
                       std::vector<std::shared_ptr<Pot>> pots,
                       int last_to_act);
    
    // Transition to a new state
    void transitionTo(std::unique_ptr<BettingState> new_state);
    
    // Handle an action
    void handleAction(const Action& action);
    
    // Getters
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    int getCurrentPlayer() const { return current_player_; }
    int getLastToAct() const { return last_to_act_; }
    int getAllInCount() const { return all_in_count_; }
    bool isRoundComplete() const { return round_complete_; }
    
    // Setters
    void setCurrentPlayer(int player) { current_player_ = player; }
    void setLastToAct(int player) { last_to_act_ = player; }
    void incrementAllInCount() { all_in_count_++; }
    void setRoundComplete(bool complete) { round_complete_ = complete; }
    
    // Helper methods
    void moveToNextPlayer();
    bool isActionValid(const Action& action) const;
    void postPlayerBets(int player_id, int amount);
    void splitPot(int pot_idx, int raise_level);
};

// Concrete states
class WaitingForActionState : public BettingState {
public:
    void enter(BettingStateMachine& machine) override;
    void exit(BettingStateMachine& machine) override;
    void handleAction(BettingStateMachine& machine, const Action& action) override;
    std::string getName() const override { return "WaitingForAction"; }
};

class ProcessingActionState : public BettingState {
private:
    Action current_action_;
    
public:
    void enter(BettingStateMachine& machine) override;
    void exit(BettingStateMachine& machine) override;
    void handleAction(BettingStateMachine& machine, const Action& action) override;
    std::string getName() const override { return "ProcessingAction"; }
};

class RoundCompleteState : public BettingState {
public:
    void enter(BettingStateMachine& machine) override;
    void exit(BettingStateMachine& machine) override;
    void handleAction(BettingStateMachine& machine, const Action& action) override;
    std::string getName() const override { return "RoundComplete"; }
}; 