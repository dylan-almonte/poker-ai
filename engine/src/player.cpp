#include "player.hpp"
#include <stdexcept>
#include <algorithm>

Player::Player(const std::string& name, int initial_chips, std::shared_ptr<Pot> pot)
    : name_(name)
    , chips_(initial_chips)
    , pot_(pot) {
}

std::unique_ptr<Action> Player::fold() {
    is_active_ = false;
    return std::make_unique<Fold>();
}

std::unique_ptr<Action> Player::call(const std::vector<std::shared_ptr<Player>>& players) {
    if (isAllIn()) {
        return std::make_unique<Call>();
    }

    int biggest_bet = 0;
    for (const auto& player : players) {
        biggest_bet = std::max(biggest_bet, player->getBetChips());
    }

    int chips_to_call = biggest_bet - getBetChips();
    addToPot(chips_to_call);
    return std::make_unique<Call>();
}

std::unique_ptr<Action> Player::raiseTo(int chips) {
    addToPot(chips);
    auto raise = std::make_unique<Raise>();
    raise->setAmount(chips);
    return raise;
}

void Player::addChips(int chips) {
    if (chips < 0) {
        throw std::invalid_argument("Cannot add negative chips");
    }
    chips_ += chips;
}

void Player::addToPot(int chips) {
    if (chips < 0) {
        throw std::invalid_argument("Cannot add negative chips to pot");
    }
    chips = tryToMakeFullBet(chips);
    pot_->addChips(this, chips);
    chips_ -= chips;
}

int Player::tryToMakeFullBet(int chips) {
    return std::min(chips, chips_);
}

void Player::addPrivateCard(std::shared_ptr<Card> card) {
    cards_.push_back(card);
} 