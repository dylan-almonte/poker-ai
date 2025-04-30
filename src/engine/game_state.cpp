#include "game_state.hpp"
#include <algorithm>
#include <stdexcept>
#include <random>

// GameStateMachine implementation
GameStateMachine::GameStateMachine(std::vector<std::shared_ptr<Player>> players,
                                 int small_blind,
                                 int big_blind)
    : players_(std::move(players))
    , current_phase_(GamePhase::PREHAND)
    , current_betting_phase_(BettingPhase::PREFLOP)
    , button_position_(0)
    , small_blind_(small_blind)
    , big_blind_(big_blind) {
    // Start in prehand state
    transitionTo(std::make_unique<PrehandState>(*this));
}

void GameStateMachine::transitionTo(std::unique_ptr<GameState> new_state) {
    if (current_state_) {
        current_state_->exit();
    }
    current_state_ = std::move(new_state);
    current_state_->enter();
}

void GameStateMachine::handleAction(const Action& action) {
    if (!current_state_) {
        throw std::runtime_error("No current state in game state machine");
    }
    current_state_->handleAction(action);
}

void GameStateMachine::moveButton() {
    do {
        button_position_ = (button_position_ + 1) % players_.size();
    } while (!players_[button_position_]->isActive());
}

void GameStateMachine::postBlinds() {
    int sb_pos = (button_position_ + 1) % players_.size();
    int bb_pos = (button_position_ + 2) % players_.size();
    
    // Post small blind
    if (players_[sb_pos]->isActive()) {
        players_[sb_pos]->setChips(players_[sb_pos]->getChips() - small_blind_);
        pots_.back()->player_post(sb_pos, small_blind_);
    }
    
    // Post big blind
    if (players_[bb_pos]->isActive()) {
        players_[bb_pos]->setChips(players_[bb_pos]->getChips() - big_blind_);
        pots_.back()->player_post(bb_pos, big_blind_);
    }
}

void GameStateMachine::dealHoleCards() {
    // Create a deck and shuffle
    std::vector<Card> deck;
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 2; rank <= 14; rank++) {
            deck.emplace_back(static_cast<Rank>(rank), static_cast<Suit>(suit));
        }
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
    
    // Deal two cards to each active player
    int card_index = 0;
    for (auto& player : players_) {
        if (player->isActive()) {
            player->setHoleCards({deck[card_index], deck[card_index + 1]});
            card_index += 2;
        }
    }
}

void GameStateMachine::dealCommunityCards() {
    // Create a deck and shuffle
    std::vector<Card> deck;
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 2; rank <= 14; rank++) {
            deck.emplace_back(static_cast<Rank>(rank), static_cast<Suit>(suit));
        }
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
    
    // Deal community cards based on current betting phase
    switch (current_betting_phase_) {
        case BettingPhase::FLOP:
            board_ = {deck[0], deck[1], deck[2]};
            break;
        case BettingPhase::TURN:
            board_.push_back(deck[3]);
            break;
        case BettingPhase::RIVER:
            board_.push_back(deck[4]);
            break;
        default:
            break;
    }
}

void GameStateMachine::startNewHand() {
    // Clear previous hand state
    pots_.clear();
    board_.clear();
    pots_.push_back(std::make_unique<Pot>());
    
    // Reset player states
    for (auto& player : players_) {
        if (player->getChips() > 0) {
            player->setState(PlayerState::IN);
        }
    }
    
    // Move button and post blinds
    moveButton();
    postBlinds();
    
    // Deal cards
    dealHoleCards();
    
    // Start betting round
    setCurrentPhase(GamePhase::BETTING_ROUND);
    setCurrentBettingPhase(BettingPhase::PREFLOP);
    transitionTo(std::make_unique<BettingRoundState>(*this));
}

void GameStateMachine::endHand() {
    // Award pots to winners
    for (auto& pot : pots_) {
        // TODO: Implement pot distribution logic
    }
    
    // Transition to hand complete state
    setCurrentPhase(GamePhase::HAND_COMPLETE);
    transitionTo(std::make_unique<HandCompleteState>(*this));
}

// PrehandState implementation
void PrehandState::enter() {
    machine_.startNewHand();
}

void PrehandState::exit() {
    // Nothing to do on exit
}

void PrehandState::handleAction(const Action& action) {
    throw std::runtime_error("Cannot handle actions in Prehand state");
}

// BettingRoundState implementation
void BettingRoundState::enter() {
    // Create new betting state machine
    int last_to_act = (machine_.getButtonPosition() + 2) % machine_.getPlayers().size();
    betting_machine_ = std::make_unique<BettingStateMachine>(
        machine_.getPlayers(),
        machine_.getPots(),
        last_to_act
    );
}

void BettingRoundState::exit() {
    // Collect all bets
    for (auto& pot : machine_.getPots()) {
        pot->collect_bets();
    }
    
    // Check if we need to move to next betting phase
    if (machine_.getCurrentBettingPhase() != BettingPhase::RIVER) {
        machine_.setCurrentBettingPhase(static_cast<BettingPhase>(
            static_cast<int>(machine_.getCurrentBettingPhase()) + 1
        ));
        machine_.dealCommunityCards();
    } else {
        // Move to showdown
        machine_.setCurrentPhase(GamePhase::SHOWDOWN);
        machine_.transitionTo(std::make_unique<ShowdownState>(machine_));
    }
}

void BettingRoundState::handleAction(const Action& action) {
    betting_machine_->handleAction(action);
    
    // Check if betting round is complete
    if (betting_machine_->isRoundComplete()) {
        exit();
    }
}

// ShowdownState implementation
void ShowdownState::enter() {
    // TODO: Implement showdown logic
    machine_.endHand();
}

void ShowdownState::exit() {
    // Nothing to do on exit
}

void ShowdownState::handleAction(const Action& action) {
    throw std::runtime_error("Cannot handle actions in Showdown state");
}

// HandCompleteState implementation
void HandCompleteState::enter() {
    // Start new hand
    machine_.setCurrentPhase(GamePhase::PREHAND);
    machine_.transitionTo(std::make_unique<PrehandState>(machine_));
}

void HandCompleteState::exit() {
    // Nothing to do on exit
}

void HandCompleteState::handleAction(const Action& action) {
    throw std::runtime_error("Cannot handle actions in HandComplete state");
} 