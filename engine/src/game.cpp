#include "game.hpp"
#include "evaluator.hpp"
#include <algorithm>
#include <stdexcept>

Game::Game(int num_players, int starting_chips, int small_blind, int big_blind)
    : btn_loc(0)
    , current_player(0)
    , phase(HandPhase::Phase::PREHAND)
    , small_blind(small_blind)
    , big_blind(big_blind) {
    
    // Initialize players
    for (int i = 0; i < num_players; i++) {
        players.push_back(std::make_shared<Player>(
            i, "Player " + std::to_string(i), starting_chips));
    }
    
    // Initialize first pot
    pots.push_back(std::make_shared<Pot>());
}

void Game::startHand() {
    // Reset game state
    board.clear();
    for (auto& player : players) {
        player->clearHand();
        player->setState(PlayerState::IN);
        player->setLastPot(0);
    }
    
    // Clear and create new pot
    pots.clear();
    pots.push_back(std::make_shared<Pot>());
    
    // Move blinds and deal cards
    moveBlinds();
    dealCards();
    postBlinds();
    
    phase = HandPhase::Phase::PREFLOP;
}

void Game::dealCards() {
    deck = Deck(); // New shuffled deck
    
    // Deal 2 cards to each player
    for (int i = 0; i < 2; i++) {
        for (auto& player : players) {
            if (player->getState() != PlayerState::SKIP) {
                auto cards = deck.draw(1);
                player->addCard(cards[0]);
            }
        }
    }
}

void Game::postBlinds() {
    int sb_pos = (btn_loc + 1) % players.size();
    int bb_pos = (btn_loc + 2) % players.size();
    
    // Post small blind
    auto& sb_player = players[sb_pos];
    int sb_amount = std::min(small_blind, sb_player->getChips());
    sb_player->setChips(sb_player->getChips() - sb_amount);
    pots[0]->player_post(sb_pos, sb_amount);
    
    if (sb_amount < small_blind) {
        sb_player->setState(PlayerState::ALL_IN);
    } else {
        sb_player->setState(PlayerState::TO_CALL);
    }
    
    // Post big blind
    auto& bb_player = players[bb_pos];
    int bb_amount = std::min(big_blind, bb_player->getChips());
    bb_player->setChips(bb_player->getChips() - bb_amount);
    pots[0]->player_post(bb_pos, bb_amount);
    
    if (bb_amount < big_blind) {
        bb_player->setState(PlayerState::ALL_IN);
    }
    
    // Set current player to UTG
    current_player = (bb_pos + 1) % players.size();
}

void Game::moveBlinds() {
    btn_loc = (btn_loc + 1) % players.size();
}

bool Game::isValidAction(ActionType action, int amount) const {
    auto player = players[current_player];
    auto current_pot = pots.back();
    int to_call = current_pot->chips_to_call(current_player);
    
    switch (action) {
        case ActionType::FOLD:
            return true;
            
        case ActionType::CHECK:
            return to_call == 0;
            
        case ActionType::CALL:
            return to_call > 0 && to_call <= player->getChips();
            
        case ActionType::RAISE:
            return amount > to_call && amount <= player->getChips();
            
        case ActionType::ALL_IN:
            return player->getChips() > 0;
            
        default:
            return false;
    }
}

void Game::handleAction(ActionType action, int amount) {
    auto player = players[current_player];
    auto current_pot = pots.back();
    
    switch (action) {
        case ActionType::FOLD:
            player->setState(PlayerState::OUT);
            break;
            
        case ActionType::CHECK:
            // No action needed
            break;
            
        case ActionType::CALL: {
            int to_call = current_pot->chips_to_call(current_player);
            player->setChips(player->getChips() - to_call);
            current_pot->player_post(current_player, to_call);
            player->setState(PlayerState::IN);
            break;
        }
            
        case ActionType::RAISE: {
            player->setChips(player->getChips() - amount);
            current_pot->player_post(current_player, amount);
            
            // Set other players to TO_CALL
            for (auto& p : players) {
                if (p->isActive() && p->getId() != current_player) {
                    p->setState(PlayerState::TO_CALL);
                }
            }
            player->setState(PlayerState::IN);
            break;
        }
            
        case ActionType::ALL_IN: {
            int chips = player->getChips();
            player->setChips(0);
            current_pot->player_post(current_player, chips);
            player->setState(PlayerState::ALL_IN);
            break;
        }
    }
    
    // Move to next active player
    current_player = getNextActivePlayer(current_player);
}

void Game::takeAction(ActionType action, int amount) {
    if (!isValidAction(action, amount)) {
        throw std::invalid_argument("Invalid action: " + actionTypeToString(action) + " with amount " + std::to_string(amount));
    }
    
    handleAction(action, amount);
    
    // Check if betting round is complete
    bool round_complete = true;
    for (const auto& player : players) {
        if (player->getState() == PlayerState::TO_CALL) {
            round_complete = false;
            break;
        }
    }
    
    if (round_complete) {
        // Collect bets and move to next phase
        for (auto& pot : pots) {
            pot->collect_bets();
        }
        
        phase = HandPhase::getNextPhase(phase);
        
        // Deal community cards if needed
        int new_cards = HandPhase::getNewCards(phase);
        if (new_cards > 0) {
            auto cards = deck.draw(new_cards);
            board.insert(board.end(), cards.begin(), cards.end());
        }
        
        // Reset current player to first active player after button
        current_player = getNextActivePlayer(btn_loc);
    }
}

std::vector<int> Game::getActivePlayers() const {
    std::vector<int> active;
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i]->isActive()) {
            active.push_back(i);
        }
    }
    return active;
}

int Game::getNextActivePlayer(int from) const {
    int next = (from + 1) % players.size();
    while (next != from) {
        if (players[next]->isActive()) {
            return next;
        }
        next = (next + 1) % players.size();
    }
    return next;
}

bool Game::isHandComplete() const {
    return phase == HandPhase::Phase::SETTLE;
}

void Game::settleHand() {
    // Evaluate hands and distribute pots
    for (auto& pot : pots) {
        std::vector<int> pot_players = pot->players_in_pot();
        if (pot_players.empty()) continue;
        
        // Find best hand
        int best_rank = 7463; // Worst possible rank + 1
        std::vector<int> winners;
        
        for (int player_id : pot_players) {
            auto& player = players[player_id];
            if (player->hasFolded()) continue;
            
            int rank = Evaluator::evaluate(player->getHand(), board);
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
            players[winner_id]->setChips(
                players[winner_id]->getChips() + amount);
        }
    }
} 