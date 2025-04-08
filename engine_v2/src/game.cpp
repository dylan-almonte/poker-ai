#include "game.hpp"
#include "evaluator.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iterator>



void Game::_deal_cards() {
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


void Game::_move_blinds() {
    btn_loc_ = (btn_loc_ + 1) % players_.size();
    
}

void Game::_settle_hand() {
    for (size_t i = 0; i < pots_.size(); i++) {
    auto& pot = pots_[i];
    std::vector<int> players_in_pot = pot->players_in_pot();

    // Only player left in pot wins
    if (players_in_pot.size() == 1) {
        players_[players_in_pot[0]]->setChips(
            players_[players_in_pot[0]]->getChips() + pot->get_total_amount()
        );
        continue;
    }

    if (board_.size() < 5) {
        auto cards = deck_.draw(5 - board_.size());
        board_.insert(board_.end(), cards.begin(), cards.end());
    }

    // Find best hand among remaining players
    std::unordered_map<int, int> player_ranks;
    int best_rank = 7463; // Worst possible rank + 1
    std::vector<int> winners;

    for (int player_id : players_in_pot) {
        if (players_[player_id]->hasFolded()) continue;

        int rank = Evaluator::evaluate(players_[player_id]->getHand(), board_);
        player_ranks[player_id] = rank;

        if (rank < best_rank) {
            best_rank = rank;
            winners.clear();
            winners.push_back(player_id);
        } else if (rank == best_rank) {
            winners.push_back(player_id);
        }
    }
    

    // Split pot among winners
    int win_amount = pot->get_total_amount() / winners.size();
    for (int winner_id : winners) {
        players_[winner_id]->setChips(
            players_[winner_id]->getChips() + win_amount
        );
    }

    // Handle leftover chips - give to first winner after button (WSOP Rule 73)
    int leftover = pot->get_total_amount() - (win_amount * winners.size());
    if (leftover > 0) {
        for (int i = btn_loc_; i < btn_loc_ + static_cast<int>(players_.size()); i++) {
            int player_id = i % players_.size();
            if (std::find(winners.begin(), winners.end(), player_id) != winners.end()) {
                players_[player_id]->setChips(
                    players_[player_id]->getChips() + leftover
                );
                break;
                }
            }
        }
    }

};


Game::Game(int num_players, int starting_chips, int small_blind, int big_blind)
    : btn_loc_(0)
    , current_player_(0)
    , small_blind_(small_blind)
    , big_blind_(big_blind)
    , betting_round_(std::make_unique<BettingRound>()) {


    // Initialize players
    for (int i = 0; i < num_players; i++) {
        auto player = std::make_shared<Player>(
            i, "Player " + std::to_string(i), starting_chips);
        players_.push_back(player);
    }

    // Initialize first pot
    pots_.push_back(std::make_shared<Pot>());
}

GameState Game::startHand(int btn_loc) {
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
        _move_blinds();
    }

    // Set blind positions
    int sb_loc = (btn_loc_ + 1) % players_.size();
    int bb_loc = (sb_loc + 1) % players_.size();

    _deal_cards();
    
    // Set initial player to act (UTG - first after BB)
    current_player_ = (bb_loc + 1) % players_.size();
    phase_ = HandPhase::Phase::PREFLOP;
    betting_round_ = std::make_unique<BettingRound>(players_, pots_, bb_loc);
    betting_round_->post_player_bets(sb_loc, small_blind_);
    betting_round_->post_player_bets(bb_loc, big_blind_);
    return getGameState();

};

GameState Game::takeAction(Action action) {
    bool next_round = false;
    
    if (HandPhase::IS_BETTING_ROUND[phase_]) {
        next_round = betting_round_->handleAction(action);
        current_player_ = betting_round_->getCurrentPlayer();
    }
    if (next_round) {
        phase_ = betting_round_->everyoneAllIn() ? HandPhase::Phase::SETTLE : HandPhase::getNextPhase(phase_);

        if (isHandOver()) {
            _settle_hand();
            GameState state = getGameState();
            state.is_terminal = true;
            for (int i = 0; i < players_.size(); i++) {
                auto player = players_[i];
                state.player_rewards[i] = player->getInitialStack() - player->getChips();
            }
            return state;
        }
        betting_round_ = std::make_unique<BettingRound>(players_, pots_, btn_loc_); 
        int new_cards = HandPhase::getNewCards(phase_);
        if (new_cards > 0) {
            auto cards = deck_.draw(new_cards);
            board_.insert(board_.end(), cards.begin(), cards.end());
        }
        current_player_ = betting_round_->getCurrentPlayer();
    }
    return getGameState();
};
bool Game::isHandOver() const {
    return phase_ == HandPhase::Phase::SETTLE;
}
bool Game::isHandComplete() const {
    return phase_ == HandPhase::Phase::SETTLE;
}


// Game state helpers

// Debug helper

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

