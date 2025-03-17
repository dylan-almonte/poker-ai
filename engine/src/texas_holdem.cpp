#include "texas_holdem.hpp"
#include "evaluator.hpp"
#include <algorithm>
#include <stdexcept>
#include <random>

TexasHoldEm::TexasHoldEm(int buyin, int big_blind, int small_blind, int max_players)
    : buyin_(buyin)
    , big_blind_(big_blind)
    , small_blind_(small_blind)
    , max_players_(max_players)
    , btn_loc_(0)
    , bb_loc_(-1)
    , sb_loc_(-1)
    , current_player_(-1)
    , last_raise_(0)
    , raise_option_(true)
    , num_hands_(0)
    , hand_phase_(HandPhase::Phase::PREHAND) {

    // Initialize players
    for (int i = 0; i < max_players_; i++) {
        auto player = std::make_shared<Player>(
            i, "Player " + std::to_string(i), buyin_);
        players_.push_back(player);
    }

    // Initialize first pot
    pots_.push_back(std::make_shared<Pot>());

    // Randomly select initial button position
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max_players - 1);
    btn_loc_ = dis(gen);
}

void TexasHoldEm::dealCards() {
    deck_ = std::make_unique<Deck>();

    // Deal 2 cards to each active player
    for (int i = 0; i < 2; i++) {
        for (auto& player : players_) {
            if (player->getState() != PlayerState::SKIP) {
                auto cards = deck_->draw(1);
                hands_[player->getId()].push_back(cards[0]);
            }
        }
    }
}

void TexasHoldEm::postBlinds() {
    // Find first active player after button for small blind
    sb_loc_ = getNextActivePlayer(btn_loc_);
    bb_loc_ = getNextActivePlayer(sb_loc_);

    // Post small blind
    auto& sb_player = players_[sb_loc_];
    int sb_amount = std::min(small_blind_, sb_player->getChips());
    playerPost(sb_loc_, sb_amount);

    if (sb_amount < small_blind_) {
        sb_player->setState(PlayerState::ALL_IN);
    } else {
        sb_player->setState(PlayerState::TO_CALL);
    }

    // Post big blind
    auto& bb_player = players_[bb_loc_];
    int bb_amount = std::min(big_blind_, bb_player->getChips());
    playerPost(bb_loc_, bb_amount);

    if (bb_amount < big_blind_) {
        bb_player->setState(PlayerState::ALL_IN);
    } else {
        bb_player->setState(PlayerState::TO_CALL);
    }

    // Set current player to UTG (Under The Gun - first to act)
    current_player_ = getNextActivePlayer(bb_loc_);
}

void TexasHoldEm::moveBlinds() {
    btn_loc_ = getNextActivePlayer(btn_loc_);
}

bool TexasHoldEm::isValidAction(Action action) const {
    return validateMove(current_player_, action.getActionType(), action.getAmount()).first;
}

std::pair<bool, std::string> TexasHoldEm::validateMove(
    int player_id,
    ActionType action,
    int total
) const {
    auto player = players_[player_id];
    int chips_to_call = chipsToCall(player_id);
    int player_amount = playerBetAmount(player_id);

    if (current_player_ != player_id) {
        return { false, "Not player's turn" };
    }

    switch (action) {
    case ActionType::FOLD:
        return { true, "" };

    case ActionType::CHECK:
        if (chips_to_call > 0) {
            return { false, "Cannot check when there's a bet to call" };
        }
        return { true, "" };

    case ActionType::CALL:
        if (chips_to_call == 0) {
            return { false, "Nothing to call" };
        }
        if (chips_to_call > player->getChips()) {
            return { false, "Not enough chips to call" };
        }
        return { true, "" };

    case ActionType::RAISE:
        if (!raise_option_) {
            return { false, "Raising not allowed at this point" };
        }
        if (total <= chips_to_call + player_amount) {
            return { false, "Raise amount must be greater than current bet" };
        }
        if (total - player_amount > player->getChips()) {
            return { false, "Not enough chips to raise" };
        }
        if (total - player_amount - chips_to_call < minRaise() &&
            total < player_amount + player->getChips()) {
            return { false, "Raise must be at least minimum raise amount" };
        }
        return { true, "" };

    case ActionType::ALL_IN:
        if (player->getChips() == 0) {
            return { false, "Player has no chips to go all-in" };
        }
        return { true, "" };

    default:
        return { false, "Invalid action type" };
    }
}

void TexasHoldEm::handleAction(Action action) {
    auto player = players_[current_player_];
    auto [action_type, total] = translateAllIn(action.getActionType(), action.getAmount());

    switch (action_type) {
    case ActionType::FOLD:
        player->setState(PlayerState::OUT);
        for (auto& pot : pots_) {
            pot->remove_player(current_player_);
        }
        break;

    case ActionType::CHECK:
        player->setState(PlayerState::IN);
        break;

    case ActionType::CALL: {
        int to_call = chipsToCall(current_player_);
        playerPost(current_player_, to_call);
        player->setState(PlayerState::IN);
        break;
    }

    case ActionType::RAISE: {
        playerPost(current_player_, total - playerBetAmount(current_player_));

        // Set other players to TO_CALL
        for (auto& p : players_) {
            if (p->isActive() && p->getId() != current_player_) {
                p->setState(PlayerState::TO_CALL);
            }
        }
        player->setState(PlayerState::IN);
        break;
    }

    case ActionType::ALL_IN: {
        int chips = player->getChips();
        playerPost(current_player_, chips);
        player->setState(PlayerState::ALL_IN);
        break;
    }
    }

    // Move to next active player
    current_player_ = getNextActivePlayer(current_player_);
}

void TexasHoldEm::playerPost(int player_id, int amount) {
    amount = std::min(players_[player_id]->getChips(), amount);
    int original_amount = amount;

    // Post to each pot the player is involved in
    for (size_t i = 0; i < pots_.size(); i++) {
        int to_call = pots_[i]->chips_to_call(player_id);
        if (amount >= to_call) {
            pots_[i]->player_post(player_id, to_call);
            amount -= to_call;
        } else {
            pots_[i]->player_post(player_id, amount);
            amount = 0;
        }

        if (amount == 0) break;
    }

    // If there's still amount left, create a new pot
    if (amount > 0) {
        auto new_pot = std::make_shared<Pot>();
        new_pot->player_post(player_id, amount);
        pots_.push_back(new_pot);
    }

    // Update player chips and last raise
    players_[player_id]->setChips(players_[player_id]->getChips() - original_amount);
    last_raise_ = std::max(last_raise_, original_amount);
}

void TexasHoldEm::splitPot(int pot_id, int raised_level) {
    auto& pot = pots_[pot_id];
    auto new_pot = std::make_shared<Pot>();

    // Move excess chips to new pot
    for (int player_id : pot->players_in_pot()) {
        int player_amount = pot->get_player_amount(player_id);
        if (player_amount > raised_level) {
            new_pot->player_post(player_id, player_amount - raised_level);
            // Adjust original pot
            // Implementation depends on Pot class interface
        }
    }

    if (!new_pot->players_in_pot().empty()) {
        pots_.insert(pots_.begin() + pot_id + 1, new_pot);
    }
}

void TexasHoldEm::startHand() {
    if (isHandRunning()) {
        throw std::runtime_error("Cannot start new hand while current hand is running");
    }

    // Reset game state
    board_.clear();
    hands_.clear();
    for (auto& player : players_) {
        player->setState(PlayerState::TO_CALL);
    }

    // Clear and create new pot
    pots_.clear();
    pots_.push_back(std::make_shared<Pot>());

    // Move blinds and deal cards
    moveBlinds();
    dealCards();
    postBlinds();

    hand_phase_ = HandPhase::Phase::PREFLOP;
    num_hands_++;
}

void TexasHoldEm::takeAction(Action action) {
    if (!isHandRunning()) {
        throw std::runtime_error("No hand is running");
    }

    if (!isValidAction(action)) {
        throw std::invalid_argument("Invalid action");
    }

    handleAction(action);

    // Check if betting round is complete
    bool round_complete = true;
    for (const auto& player : players_) {
        if (player->getState() == PlayerState::TO_CALL) {
            round_complete = false;
            break;
        }
    }

    if (round_complete || isHandOver()) {
        // Collect bets and move to next phase
        for (auto& pot : pots_) {
            pot->collect_bets();
        }

        hand_phase_ = HandPhase::getNextPhase(hand_phase_);

        // Deal community cards if needed
        int new_cards = HandPhase::getNewCards(hand_phase_);
        if (new_cards > 0) {
            auto cards = deck_->draw(new_cards);
            board_.insert(board_.end(), cards.begin(), cards.end());
        }

        // Reset player states for new betting round
        for (auto& player : players_) {
            if (player->getState() == PlayerState::IN) {
                player->setState(PlayerState::TO_CALL);
            }
        }

        // Reset betting round state
        last_raise_ = 0;
        raise_option_ = true;
        current_player_ = getNextActivePlayer(btn_loc_);
    }
}

void TexasHoldEm::settleHand() {
    if (hand_phase_ != HandPhase::Phase::SETTLE) {
        throw std::runtime_error("Not time for settlement");
    }

    // Make sure there are 5 cards on the board for all-in situations
    while (board_.size() < 5) {
        auto cards = deck_->draw(1);
        board_.insert(board_.end(), cards.begin(), cards.end());
    }

    // Evaluate and distribute each pot
    for (auto& pot : pots_) {
        std::vector<int> pot_players = pot->players_in_pot();
        if (pot_players.empty()) continue;

        // Find best hand
        int best_rank = 7463; // Worst possible rank + 1
        std::vector<int> winners;

        for (int player_id : pot_players) {
            if (players_[player_id]->hasFolded()) continue;

            int rank = Evaluator::evaluate(hands_[player_id], board_);
            if (rank < best_rank) {
                best_rank = rank;
                winners.clear();
                winners.push_back(player_id);
            } else if (rank == best_rank) {
                winners.push_back(player_id);
            }
        }

        // Split pot among winners
        int amount = pot->get_total_amount() / winners.size();
        for (int winner_id : winners) {
            players_[winner_id]->setChips(
                players_[winner_id]->getChips() + amount
            );
        }

        // Handle leftover chips - give to first winner after button
        int leftover = pot->get_total_amount() - (amount * winners.size());
        if (leftover > 0) {
            for (int i = btn_loc_; i < btn_loc_ + max_players_; i++) {
                int player_id = i % max_players_;
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

bool TexasHoldEm::isHandOver() const {
    // Count active non-all-in players
    int active_count = 0;
    for (const auto& player : players_) {
        if (player->isActive() && !player->isAllIn()) {
            active_count++;
        }
    }
    return active_count <= 1;
}

bool TexasHoldEm::isHandRunning() const {
    return hand_phase_ != HandPhase::Phase::PREHAND;
}

bool TexasHoldEm::isHandComplete() const {
    return hand_phase_ == HandPhase::Phase::SETTLE;
}

std::vector<int> TexasHoldEm::getActivePlayers() const {
    std::vector<int> active;
    for (size_t i = 0; i < players_.size(); i++) {
        if (players_[i]->isActive()) {
            active.push_back(i);
        }
    }
    return active;
}

int TexasHoldEm::getNextActivePlayer(int from) const {
    for (int i = 1; i <= max_players_; i++) {
        int next = (from + i) % max_players_;
        if (players_[next]->isActive()) {
            return next;
        }
    }
    return from; // No other active players found
}

int TexasHoldEm::chipsToCall(int player_id) const {
    int total = 0;
    for (const auto& pot : pots_) {
        total += pot->chips_to_call(player_id);
    }
    return total;
}

int TexasHoldEm::playerBetAmount(int player_id) const {
    int total = 0;
    for (const auto& pot : pots_) {
        total += pot->get_player_amount(player_id);
    }
    return total;
}

int TexasHoldEm::chipsAtStake(int player_id) const {
    int total = 0;
    for (const auto& pot : pots_) {
        if (std::find(pot->players_in_pot().begin(),
            pot->players_in_pot().end(),
            player_id) != pot->players_in_pot().end()) {
            total += pot->get_total_amount();
        }
    }
    return total;
}

int TexasHoldEm::minRaise() const {
    return std::max(big_blind_, last_raise_);
}

std::pair<ActionType, int> TexasHoldEm::translateAllIn(
    ActionType action,
    int total
) const {
    if (action != ActionType::ALL_IN) {
        return { action, total };
    }

    auto player = players_[current_player_];
    int to_call = chipsToCall(current_player_);

    if (player->getChips() <= to_call) {
        return { ActionType::CALL, 0 };
    }

    return {
        ActionType::RAISE,
        playerBetAmount(current_player_) + player->getChips()
    };
}

int TexasHoldEm::previousAllInSum() const {
    int sum = 0;
    for (const auto& player : players_) {
        if (player->isAllIn()) {
            sum += playerBetAmount(player->getId());
        }
    }
    return sum;
}

const std::vector<Card>& TexasHoldEm::getHand(int player_id) const {
    static const std::vector<Card> empty_hand;
    auto it = hands_.find(player_id);
    return it != hands_.end() ? it->second : empty_hand;
}

// ... To be continued with settlement and helper methods ... 