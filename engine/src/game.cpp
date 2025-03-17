#include "game.hpp"
#include "evaluator.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

Game::Game(int num_players, int starting_chips, int small_blind, int big_blind)
    : btn_loc_(0)
    , current_player_(0)
    , phase_(HandPhase::Phase::PREHAND)
    , small_blind_(small_blind)
    , big_blind_(big_blind) {
    
    // Initialize players
    for (int i = 0; i < num_players; i++) {
        auto player = std::make_shared<Player>(
            i, "Player " + std::to_string(i), starting_chips);
        players_.push_back(player);
    }
    
    // Initialize first pot
    pots_.push_back(std::make_shared<Pot>());
}

void Game::startHand(int btn_loc) {
    
    // Reset game state
    board_.clear();
    for (auto& player : players_) {
        player->clearHand();
        player->setState(PlayerState::TO_CALL);
        player->setLastPot(0);
        player->setInitialStack(player->getChips());
    }
    
    // Clear and create new pot
    pots_.clear();
    pots_.push_back(std::make_shared<Pot>());
    
    // Move blinds and deal cards
    if (btn_loc != -1) {
        btn_loc_ = btn_loc;
    } else {    
        moveBlinds();
    }
    dealCards();
    postBlinds();
    
    phase_ = HandPhase::Phase::PREFLOP;
}

void Game::dealCards() {
    deck_ = Deck(); // New shuffled deck
    
    // Deal 2 cards to each player
    for (int i = 0; i < 2; i++) {
        for (auto& player : players_) {
            if (player->getState() != PlayerState::SKIP) {
                auto cards = deck_.draw(1);
                player->addCard(cards[0]);
            }
        }
    }
}

void Game::postBlinds() {
    int sb_pos = (btn_loc_ + 1) % players_.size();
    int bb_pos = (btn_loc_ + 2) % players_.size();
    
    // Post small blind
    auto& sb_player = players_[sb_pos];
    int sb_amount = std::min(small_blind_, sb_player->getChips());
    sb_player->setChips(sb_player->getChips() - sb_amount);
    pots_[0]->player_post(sb_pos, sb_amount);
    
    if (sb_amount < small_blind_) {
        sb_player->setState(PlayerState::ALL_IN);
    } else {
        sb_player->setState(PlayerState::TO_CALL);
    }
    
    // Post big blind
    auto& bb_player = players_[bb_pos];
    int bb_amount = std::min(big_blind_, bb_player->getChips());
    bb_player->setChips(bb_player->getChips() - bb_amount);
    pots_[0]->player_post(bb_pos, bb_amount);
    bb_player->setState(PlayerState::TO_CALL);
    if (bb_amount < big_blind_) {
        bb_player->setState(PlayerState::ALL_IN);
    }
    
    // Set current player to UTG
    // set last player to BB
    last_player_ = bb_pos;
    current_player_ = (bb_pos + 1) % players_.size();
}

void Game::moveBlinds() {
    btn_loc_ = (btn_loc_ + 1) % players_.size();
}

bool Game::isValidAction(Action action) const {
    auto player = players_[current_player_];
    auto current_pot = pots_.back();
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

void Game::handleAction(Action action) {
    auto player = players_[current_player_];
    auto current_pot = pots_.back();
    action = translateAllIn(action);
    action_history_.push_back(action);

    switch (action.getActionType()) {
        case ActionType::FOLD:
            player->setState(PlayerState::OUT);
            break;
            
        case ActionType::CHECK:
            // No action needed
            player->setState(PlayerState::IN);

            break;
            
        case ActionType::CALL: {
            int to_call = current_pot->chips_to_call(current_player_);
            player->setChips(player->getChips() - to_call);
            current_pot->player_post(current_player_, to_call);
            player->setState(PlayerState::IN);
            break;
        }
            
        case ActionType::RAISE: {
            player->setChips(player->getChips() - action.getAmount());
            current_pot->player_post(current_player_, action.getAmount());
            
            // Set other players to TO_CALL
            for (auto& p : players_) {
                if (p->isActive() && p->getId() != current_player_ && !p->isAllIn()) {
                    p->setState(PlayerState::TO_CALL);
                }
            }
            player->setState(PlayerState::IN);
            break;
        }
            
        case ActionType::ALL_IN: {
            int chips = player->getChips();
            player->setChips(0);
            current_pot->player_post(current_player_, chips);
            player->setState(PlayerState::ALL_IN);
            break;
        }
    }
    
    // Move to next active player
    current_player_ = getNextActivePlayer(current_player_);
}

void Game::takeAction(Action action) {
    // if (!isValidAction(action)) {
    //     printState();
    //     throw std::invalid_argument("Invalid action: " + actionTypeToString(action.getActionType()) + " with amount " + std::to_string(action.getAmount()));
    // }
    
    handleAction(action);
    
    // Check if betting round is complete
    bool round_complete = true;
    for (const auto& player : players_) {
        if (player->getState() == PlayerState::TO_CALL) {
            round_complete = false;
            break;
        }
    }
    int active_count = 0;
    for (const auto& player : players_) {
        if (player->isActive() && !player->isAllIn()) {
            active_count++;
        }
    }
    bool hand_over = active_count <= 1;
    
    if (round_complete || hand_over) {
        // Collect bets and move to next phase
        for (auto& pot : pots_) {
            pot->collect_bets();
        }
        
        phase_ = HandPhase::getNextPhase(phase_);
        
        // Deal community cards if needed
        int new_cards = HandPhase::getNewCards(phase_);
        if (new_cards > 0) {
            auto cards = deck_.draw(new_cards);
            board_.insert(board_.end(), cards.begin(), cards.end());
        }
        for (const auto& player : players_) {
            if (player->getState() == PlayerState::IN) {
                player->setState(PlayerState::TO_CALL);
            }
        }
        
        // Reset current player to first active player after button
        current_player_ = getNextActivePlayer(btn_loc_);
    }
}

std::vector<int> Game::getActivePlayers() const {
    std::vector<int> active;
    for (size_t i = 0; i < players_.size(); i++) {
        if (players_[i]->isActive()) {
            active.push_back(i);
        }
    }
    return active;
}

int Game::getNextActivePlayer(int from) const {
    int next = (from + 1) % players_.size();
    while (next != from) {
        if (players_[next]->isActive()) {
            return next;
        }
        next = (next + 1) % players_.size();
    }
    return next;
}

bool Game::isHandComplete() const {
    return phase_ == HandPhase::Phase::SETTLE;
}

bool Game::isHandOver() const {
    int active_count = 0;
    for (const auto& player : players_) {
        if (player->isActive() && !player->isAllIn()) {
            active_count++;
        }
    }
    return active_count <= 1;
}

void Game::settleHand() {

    // Evaluate hands and distribute pots
    for (auto& pot : pots_) {
        std::vector<int> pot_players = pot->players_in_pot();
        if (pot_players.empty()) continue;
        
        // Find best hand
        int best_rank = 7463; // Worst possible rank + 1
        std::vector<int> winners;
        
        for (int player_id : pot_players) {
            auto& player = players_[player_id];
            if (player->hasFolded()) continue;
            
            int rank = Evaluator::evaluate(player->getHand(), board_);
            if (rank < best_rank) {
                best_rank = rank;
                winners.clear();
                winners.push_back(player_id);
            } else if (rank == best_rank) {
                winners.push_back(player_id);
            }
        }
        
        // Split pot among winners
        int amount = pot->get_amount() / winners.size();
        for (int winner_id : winners) {
            players_[winner_id]->setChips(
                players_[winner_id]->getChips() + amount);
        }
    }
}
float Game::getPayoff(int player_idx) const {
    // Get the player's initial stack at the start of the hand
    int initial_stack = players_[player_idx]->getInitialStack();
    
    // Get their current stack
    int current_stack = players_[player_idx]->getChips();
    
    // The payoff is the difference (positive if they won money, negative if they lost)
    return static_cast<float>(current_stack - initial_stack);
}

void Game::printState() const {
    std::cout << "\n=== Game State ===\n";
    
    // Print phase
    std::cout << "Phase: " << phaseToString(phase_) << "\n";
    
    // Print board
    std::cout << "Board: ";
    if (board_.empty()) {
        std::cout << "[]";
    } else {
        std::cout << prettyPrintCards(board_);
    }
    std::cout << "\n";
    
    // Print pots
    std::cout << "Pots: ";
    for (size_t i = 0; i < pots_.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << "Pot " << i << ": $" << pots_[i]->get_total_amount();
    }
    std::cout << "\n";
    
    // Print players
    std::cout << "\nPlayers:\n";
    for (size_t i = 0; i < players_.size(); i++) {
        const auto& player = players_[i];
        std::cout << (i == static_cast<size_t>(current_player_) ? "→ " : "  ");
        std::cout << "Player " << i 
                  << " ($" << player->getChips() << "): "
                  << playerStateToString(player->getState())
                  << (i == static_cast<size_t>(btn_loc_) ? " [BTN] " : "       ");
        
        // Only show cards for active players
        if (player->isActive() || player->isAllIn()) {
            std::cout << " " << prettyPrintCards(player->getHand());
            std::cout << " " << pots_[pots_.size() - 1]->get_player_amount(i);
        }
        
        // Show button position
        // if (i == static_cast<size_t>(btn_loc)) {
        //     std::cout << " [BTN]";
        // }
        
        std::cout << "\n";
    }
    std::cout << "\n";
} 

int Game::getInitialStackTotal() const {
    int total = 0;
    for (const auto& player : players_) {
        total += player->getInitialStack();
    }
    return total;
}

Action Game::translateAllIn(Action action) {
    const auto& player = players_[current_player_];
    if (action.getActionType() != ActionType::ALL_IN) {
        return action;
    }
    auto current_pot = pots_.back();
    int to_call = current_pot->chips_to_call(current_player_);

    if (player->getChips() <= to_call) {
        return { ActionType::CALL, 0 };
    }

    return {
        ActionType::RAISE,
        player->getChips()
    };
}

