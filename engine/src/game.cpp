#include "game.hpp"
#include "evaluator.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iterator>

Game::Game(int num_players, int starting_chips, int small_blind, int big_blind)
    : btn_loc_(0)
    , current_player_(0)
    , small_blind_(small_blind)
    , big_blind_(big_blind)
    , action_awaiter_() {

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
        _moveBlinds();
    }

    // Set blind positions
    sb_loc_ = (btn_loc_ + 1) % players_.size();
    bb_loc_ = (sb_loc_ + 1) % players_.size();

    _dealCards();
    _postPlayerBets(sb_loc_, small_blind_);
    _postPlayerBets(bb_loc_, big_blind_);

    // Set initial player to act (UTG - first after BB)
    current_player_ = (bb_loc_ + 1) % players_.size();
    phase_machine_.setPhase(HandPhase::Phase::PREFLOP);
}

/**************************************************
 * Private methods
 **************************************************/


void Game::_dealCards() {
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
void Game::_postPlayerBets(int player_idx, int amount) {

    amount = std::min(amount, players_[player_idx]->getChips());
    int original_amount = amount;
    int last_pot_idx = players_[player_idx]->getLastPot();

    // if a player posts, they are in the pot
    if (amount == players_[player_idx]->getChips()) {
        players_[player_idx]->setState(PlayerState::ALL_IN);
    } else {
        players_[player_idx]->setState(PlayerState::IN);
    }

    for (int i = 0; i < last_pot_idx; i++) {
        amount = amount - pots_[i]->chips_to_call(player_idx);
        pots_[i]->player_post(player_idx, pots_[i]->chips_to_call(player_idx));
    }

    int prev_raise_level = pots_[last_pot_idx]->get_raised();
    pots_[last_pot_idx]->player_post(player_idx, amount);

    int last_raise = pots_[last_pot_idx]->get_raised() - prev_raise_level;
    last_raise = std::max(last_raise, last_raise_);

    auto pot_players = pots_[last_pot_idx]->players_in_pot();

    // players previously in pot need to call in event of a raise
    if (last_raise > 0) {
        for (int pot_player_id : pot_players) {
            if (pots_[last_pot_idx]->chips_to_call(pot_player_id) > 0 && players_[pot_player_id]->getState() == PlayerState::IN) {
                players_[pot_player_id]->setState(PlayerState::TO_CALL);
            }
        }
    }

    // if a player is all_in in this pot, split a new one off
    bool all_in_exists = false;
    for (int pot_player_id : pot_players) {
        if (players_[pot_player_id]->getState() == PlayerState::ALL_IN) {
            all_in_exists = true;
        }
    }
    if (all_in_exists) {
        int new_raise_level = INT_MAX;
        auto active_players = getActivePlayers();
        auto last_pot = pots_[last_pot_idx];
        for (int pot_player_id : active_players) {
            if (last_pot->get_player_amount(pot_player_id) > new_raise_level) {
                new_raise_level = std::min(new_raise_level, last_pot->get_player_amount(pot_player_id));
            }
        }
        _splitPot(last_pot_idx, new_raise_level);
    }
    players_[player_idx]->setChips(players_[player_idx]->getChips() - original_amount);
}

void Game::_splitPot(int pot_idx, int raise_level) {
    auto pot = pots_[pot_idx];
    if (pot->get_raised() <= raise_level) {
        return;
    }
    Pot split_pot;
    auto pot_players = pots_[pot_idx]->players_in_pot();

    // TODO: maybe optimize this
    pots_.insert(pots_.begin() + pot_idx + 1, std::make_shared<Pot>(split_pot));

    for (int pot_player_id : pot_players) {
        if (pot->get_player_amount(pot_player_id) > raise_level) {
            split_pot.player_post(pot_player_id, pot->get_player_amount(pot_player_id) - raise_level);
        }
    }

    auto active_players_ = getActivePlayers();
    for (int player_id : active_players_) {
        if (players_[player_id]->getChips() > getTotalToCall(player_id)) {
            players_[player_id]->setLastPot(pot_idx + 1);
        }
    }
}



void Game::_moveBlinds() {
    btn_loc_ = (btn_loc_ + 1) % players_.size();
}

bool Game::_isValidAction(Action action) const {
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

bool Game::_isHandOver() const {
    int count = 0;
    for (const auto& player : players_) {
        if (player->getState() == PlayerState::TO_CALL) {
            return false;
        }
        if (player->getState() == PlayerState::IN) {
            count++;
        }
        if (count > 1) {
            return false;
        }
    }
    return true;
}

void Game::_handleAction(Action action) {
    auto player = players_[current_player_];
    auto current_pot = pots_.back();
    action = _translateAllIn(action);
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

Action Game::_translateAllIn(Action action) {
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



void Game::_settleHand() {

    // Evaluate and distribute each pot
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
}


void Game::_takeAction(Action action) {
    if (action.getActionType() == ActionType::ALL_IN) {
        action = _translateAllIn(action);
    }

    int player_amount = players_[current_player_]->getChips();
    int to_call = getTotalToCall(current_player_);

    switch (action.getActionType()) {
    case ActionType::CALL:
        _postPlayerBets(current_player_, to_call);
        break;
    case ActionType::RAISE:
        _postPlayerBets(current_player_, action.getAmount() - player_amount);
        break;
    case ActionType::FOLD:
        players_[current_player_]->setState(PlayerState::OUT);
        for (int i = 0; i < players_[current_player_]->getLastPot() + 1; i++) {
            pots_[i]->remove_player(current_player_);
        }
        break;
    }

}

BettingRoundCoroutine Game::_bettingRound(HandPhase::Phase phase) {
    // Add new cards to the board based on the phase
    int new_cards = HandPhase::getNewCards(phase);
    if (new_cards > 0) {
        auto cards = deck_.draw(new_cards);
        board_.insert(board_.end(), cards.begin(), cards.end());
    }

    // Reset betting state for this round
    int first_pot_idx = pots_.size() - 1;
    last_raise_ = 0;
    raise_option_ = true;

    // Set initial player position
    // For preflop, first player is after BB
    // For all other rounds, first player is after button
    current_player_ = (phase == HandPhase::Phase::PREFLOP)
        ? (bb_loc_ + 1) % players_.size()
        : (btn_loc_ + 1) % players_.size();

    // Get initial queue of active players
    auto active_players = _player_iter(current_player_);
    std::deque<int> player_queue(active_players.begin(), active_players.end());

    while (!isHandOver()) {
        // WSOP 2021 Rule 96:
        // If no more active players can raise, continue with players who need to call
        // while disabling the raise availability
        if (player_queue.empty()) {
            auto to_call_players = _player_iter(
                current_player_ + 1,
                false,
                { PlayerState::TO_CALL }
            );
            player_queue = std::deque<int>(to_call_players.begin(), to_call_players.end());

            if (player_queue.empty()) {
                break;
            }
            raise_option_ = false;
        }

        // Store previous raise amount for Rule 96
        int prev_raised = last_raise_;

        // Set current player and process their action
        current_player_ = player_queue.front();
        player_queue.pop_front();

        // Suspend execution until takeAction is called
        Action action = co_await action_awaiter_;

        // If a raise occurred, everyone eligible gets another action
        if (!action_history_.empty() && action_history_.back().getActionType() == ActionType::RAISE) {
            int raise_amount = action_history_.back().getAmount();

            // WSOP Rule 96: An all-in raise less than the previous raise shall not reopen
            // the bidding unless two or more such all-in raises total greater than or equal
            // to the previous raise
            int all_in_raise_sum = 0;
            for (auto it = action_history_.rbegin(); it != action_history_.rend(); ++it) {
                if (players_[it->getPlayerId()]->isAllIn() &&
                    it->getActionType() == ActionType::RAISE) {
                    all_in_raise_sum += it->getAmount();
                } else if (!players_[it->getPlayerId()]->isAllIn()) {
                    break;
                }
            }

            if (raise_amount < prev_raised) {
                if (all_in_raise_sum < prev_raised) {
                    continue;
                }
                // Exception for rule 96
                last_raise_ = all_in_raise_sum;
            }

            // Reset the round (as if betting round started here)
            auto new_active_players = _player_iter(current_player_);
            player_queue = std::deque<int>(new_active_players.begin(), new_active_players.end());

            // Remove current player from queue if they're not all-in
            if (!players_[current_player_]->isAllIn() && !player_queue.empty()) {
                player_queue.pop_front();
            }
        }
    }

    // Consolidate betting for all pots in this betting round
    for (size_t i = first_pot_idx; i < pots_.size(); i++) {
        pots_[i]->collect_bets();
    }
}


std::vector<int> Game::_player_iter(
        std::optional<int> loc,
        bool reverse,
        std::vector<PlayerState> match_states,
        std::vector<PlayerState> filter_states) {

    // Get starting location (default to current player if not specified)
    int start_loc = loc.value_or(current_player_);

    // Set up iteration range
    int start = start_loc;
    int stop = start_loc + players_.size();
    int step = 1;

    if (reverse) {
        std::swap(start, stop);
        step = -1;
    }

    // Collect valid player IDs
    std::vector<int> valid_players;
    for (int i = start; i != stop; i += step) {
        int player_idx = i % players_.size();

        // Check if player state matches criteria
        PlayerState state = players_[player_idx]->getState();

        // Skip if state is in filter_states
        if (std::find(filter_states.begin(), filter_states.end(), state)
            != filter_states.end()) {
            continue;
        }

        // Skip if match_states is not empty and state is not in match_states
        if (!match_states.empty() &&
            std::find(match_states.begin(), match_states.end(), state)
            == match_states.end()) {
            continue;
        }

        valid_players.push_back(player_idx);
    }

    return valid_players;
}

/**************************************************
 * Public methods
 **************************************************/

void Game::takeAction(Action action) {
    if (!_isValidAction(action)) {
        throw std::invalid_argument("Invalid action");
    }

    _handleAction(action);
    action_history_.push_back(action);

    // Resume the betting round coroutine if it exists
    if (current_betting_round_) {
        action_awaiter_.action = action;
        current_betting_round_.resume();
    }

    // Check if betting round is complete
    bool round_complete = true;
    for (const auto& player : players_) {
        if (player->getState() == PlayerState::TO_CALL) {
            round_complete = false;
            break;
        }
    }

    bool hand_over = isHandOver();
    if (round_complete || hand_over) {
        // Clean up current betting round
        if (current_betting_round_) {
            current_betting_round_.destroy();
            current_betting_round_ = nullptr;
        }

        // Collect bets from all pots
        for (auto& pot : pots_) {
            pot->collect_bets();
        }

        // Use state machine to handle phase transition
        phase_machine_.transition(round_complete, hand_over);

        // Start new betting round if needed
        if (phase_machine_.getCurrentPhase() != HandPhase::Phase::SETTLE) {
            auto betting_round = _bettingRound(phase_machine_.getCurrentPhase());
            current_betting_round_ = betting_round.handle;
        } else {
            _settleHand();
        }
    }
}

std::vector<int> Game::getActivePlayers(int loc) const {
    std::vector<int> active;

    for (size_t i = 0; i < players_.size(); i++) {
        size_t idx = (i + loc) % players_.size();
        if (players_[idx]->isActive()) {
            active.push_back(idx);
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

// bool Game::isHandComplete() const {
//     return phase_ == HandPhase::Phase::SETTLE;
// }

bool Game::isHandOver() const {
    // Hand is over if:
    // 1. Only one active player remains (everyone else folded)
    // 2. We've reached showdown (river betting complete)
    // 3. Everyone is all-in

    if (phase_machine_.getCurrentPhase() == HandPhase::Phase::SETTLE) {
        return true;
    }

    int active_count = 0;
    int all_in_count = 0;
    for (const auto& player : players_) {
        if (player->isActive() && !player->isAllIn()) {
            active_count++;
        }
        if (player->isAllIn()) {
            all_in_count++;
        }
    }

    // Only one active player (rest folded)
    if (active_count <= 1) {
        return true;
    }

    // Everyone is either all-in or folded
    if (active_count == 0 && all_in_count > 0) {
        return true;
    }

    // Reached showdown
    if (phase_machine_.getCurrentPhase() == HandPhase::Phase::RIVER && active_count > 0) {
        bool betting_complete = true;
        for (const auto& player : players_) {
            if (player->getState() == PlayerState::TO_CALL) {
                betting_complete = false;
                break;
            }
        }
        return betting_complete;
    }

    return false;
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
    std::cout << "Phase: " << phaseToString(phase_machine_.getCurrentPhase()) << "\n";

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

int Game::getTotalToCall(int player_id) const {
    int to_call = 0;
    for (int i = 0; i < players_[player_id]->getLastPot() + 1; i++) {
        to_call += pots_[i]->chips_to_call(player_id);
    }
    return to_call;
}