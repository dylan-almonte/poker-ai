#include "betting_state.hpp"
#include <algorithm>
#include <stdexcept>

// BettingStateMachine implementation
BettingStateMachine::BettingStateMachine(std::vector<std::shared_ptr<Player>> players,
                                       std::vector<std::shared_ptr<Pot>> pots,
                                       int last_to_act)
    : players_(std::move(players))
    , pots_(std::move(pots))
    , current_player_((last_to_act + 1) % players_.size())
    , last_to_act_(last_to_act)
    , all_in_count_(0)
    , round_complete_(false) {
    // Start in waiting state
    transitionTo(std::make_unique<WaitingForActionState>());
}

void BettingStateMachine::transitionTo(std::unique_ptr<BettingState> new_state) {
    if (current_state_) {
        current_state_->exit(*this);
    }
    current_state_ = std::move(new_state);
    current_state_->enter(*this);
}

void BettingStateMachine::handleAction(const Action& action) {
    if (!current_state_) {
        throw std::runtime_error("No current state in betting state machine");
    }
    current_state_->handleAction(*this, action);
}

void BettingStateMachine::moveToNextPlayer() {
    do {
        current_player_ = (current_player_ + 1) % players_.size();
    } while (!players_[current_player_]->isActive());
}

bool BettingStateMachine::isActionValid(const Action& action) const {
    const auto& player = players_[current_player_];
    const auto& current_pot = pots_.back();
    int to_call = current_pot->chips_to_call(current_player_);
    
    switch (action.getActionType()) {
        case ActionType::FOLD:
            return true;
        case ActionType::CHECK:
            return to_call == 0;
        case ActionType::CALL:
            return to_call >= 0 && to_call <= player->getChips();
        case ActionType::RAISE:
            return action.getAmount() > to_call && action.getAmount() <= player->getChips();
        case ActionType::ALL_IN:
            return player->getChips() > 0;
        default:
            return false;
    }
}

void BettingStateMachine::postPlayerBets(int player_id, int amount) {
    auto player = players_[player_id];
    amount = std::min(amount, player->getChips());
    int original_amount = amount;
    int last_pot_idx = player->getLastPot();

    // Handle all-in
    if (amount >= player->getChips()) {
        player->setState(PlayerState::ALL_IN);
        incrementAllInCount();
    } else {
        player->setState(PlayerState::IN);
    }

    // Post bets to previous pots
    for (int i = 0; i < last_pot_idx; i++) {
        amount = amount - pots_[i]->chips_to_call(player_id);
        pots_[i]->player_post(player_id, pots_[i]->chips_to_call(player_id));
    }

    // Post to current pot
    int prev_raise_level = pots_[last_pot_idx]->get_raised();
    pots_[last_pot_idx]->player_post(player_id, amount);

    // Handle raises
    int last_raise = pots_[last_pot_idx]->get_raised() - prev_raise_level;
    if (last_raise > 0) {
        setLastToAct(current_player_);
        for (const auto& pot_player : pots_[last_pot_idx]->players_in_pot()) {
            if (pots_[last_pot_idx]->chips_to_call(pot_player) > 0 && 
                players_[pot_player]->getState() == PlayerState::IN) {
                players_[pot_player]->setState(PlayerState::TO_CALL);
            }
        }
    }

    // Handle side pots
    bool all_in_exists = false;
    for (const auto& pot_player : pots_[last_pot_idx]->players_in_pot()) {
        if (players_[pot_player]->getState() == PlayerState::ALL_IN) {
            all_in_exists = true;
            break;
        }
    }
    
    if (all_in_exists) {
        int new_raise_level = INT_MAX;
        for (const auto& pot_player : pots_[last_pot_idx]->players_in_pot()) {
            if (players_[pot_player]->getState() == PlayerState::ALL_IN) {
                new_raise_level = std::min(new_raise_level, 
                    pots_[last_pot_idx]->get_player_amount(pot_player));
            }
        }
        splitPot(last_pot_idx, new_raise_level);
    }

    player->setChips(player->getChips() - original_amount);
}

void BettingStateMachine::splitPot(int pot_idx, int raise_level) {
    auto pot = pots_[pot_idx];
    if (pot->get_raised() <= raise_level) {
        return;
    }

    auto split_pot = std::make_unique<Pot>();
    auto pot_players = pot->players_in_pot();

    for (const auto& player_id : pot_players) {
        if (pot->get_player_amount(player_id) > raise_level) {
            split_pot->player_post(player_id, pot->get_player_amount(player_id) - raise_level);
        }
    }

    pots_.insert(pots_.begin() + pot_idx + 1, std::move(split_pot));

    for (size_t player_id = 0; player_id < players_.size(); player_id++) {
        if (players_[player_id]->isActive() && 
            players_[player_id]->getChips() > chipsToCall(player_id)) {
            players_[player_id]->setLastPot(pot_idx + 1);
        }
    }
}

// WaitingForActionState implementation
void WaitingForActionState::enter(BettingStateMachine& machine) {
    // Nothing to do on enter
}

void WaitingForActionState::exit(BettingStateMachine& machine) {
    // Nothing to do on exit
}

void WaitingForActionState::handleAction(BettingStateMachine& machine, const Action& action) {
    if (!machine.isActionValid(action)) {
        throw std::invalid_argument("Invalid action for current state");
    }
    
    machine.transitionTo(std::make_unique<ProcessingActionState>());
    machine.handleAction(action);
}

// ProcessingActionState implementation
void ProcessingActionState::enter(BettingStateMachine& machine) {
    // Nothing to do on enter
}

void ProcessingActionState::exit(BettingStateMachine& machine) {
    // Nothing to do on exit
}

void ProcessingActionState::handleAction(BettingStateMachine& machine, const Action& action) {
    auto& player = machine.getPlayers()[machine.getCurrentPlayer()];
    auto& current_pot = machine.getPots().back();
    
    switch (action.getActionType()) {
        case ActionType::FOLD:
            player->setState(PlayerState::OUT);
            break;
        case ActionType::CHECK:
            player->setState(PlayerState::IN);
            break;
        case ActionType::CALL:
            machine.postPlayerBets(machine.getCurrentPlayer(), 
                                 current_pot->chips_to_call(machine.getCurrentPlayer()));
            break;
        case ActionType::RAISE:
            machine.postPlayerBets(machine.getCurrentPlayer(), action.getAmount());
            break;
        case ActionType::ALL_IN:
            machine.postPlayerBets(machine.getCurrentPlayer(), player->getChips());
            break;
    }

    // Check if round is complete
    if (machine.getCurrentPlayer() == machine.getLastToAct() || 
        machine.getAllInCount() == machine.getPlayers().size() - 1) {
        machine.setRoundComplete(true);
        machine.transitionTo(std::make_unique<RoundCompleteState>());
    } else {
        machine.moveToNextPlayer();
        machine.transitionTo(std::make_unique<WaitingForActionState>());
    }
}

// RoundCompleteState implementation
void RoundCompleteState::enter(BettingStateMachine& machine) {
    // Nothing to do on enter
}

void RoundCompleteState::exit(BettingStateMachine& machine) {
    // Nothing to do on exit
}

void RoundCompleteState::handleAction(BettingStateMachine& machine, const Action& action) {
    throw std::runtime_error("Cannot handle actions in RoundComplete state");
} 