#include "pot.hpp"
#include <numeric>
#include <algorithm>

int Pot::chips_to_call(int player_id) const {
    auto it = player_amounts_.find(player_id);
    return raised_ - (it != player_amounts_.end() ? it->second : 0);
}

void Pot::player_post(int player_id, int amount) {
    player_amounts_[player_id] = player_amounts_[player_id] + amount;
    if (player_amounts_[player_id] > raised_) {
        raised_ = player_amounts_[player_id];
    }
}

Pot Pot::with_player_post(int player_id, int amount) const {
    Pot new_pot = *this;
    new_pot.player_amounts_[player_id] = player_amounts_.count(player_id) ?
        player_amounts_.at(player_id) + amount : amount;

    if (new_pot.player_amounts_[player_id] > raised_) {
        new_pot.raised_ = new_pot.player_amounts_[player_id];
    }

    return new_pot;
}

int Pot::get_player_amount(int player_id) const {
    auto it = player_amounts_.find(player_id);
    return it != player_amounts_.end() ? it->second : 0;
}

std::optional<int> Pot::get_player_amount_optional(int player_id) const {
    auto it = player_amounts_.find(player_id);
    return it != player_amounts_.end() ? std::optional<int>(it->second) : std::nullopt;
}

std::vector<int> Pot::players_in_pot() const {
    std::vector<int> players;
    players.reserve(player_amounts_.size());
    for (const auto& [player_id, _] : player_amounts_) {
        players.push_back(player_id);
    }
    return players;
}

void Pot::collect_bets() {
    for (auto& [player_id, player_amount] : player_amounts_) {
        amount_ += player_amount;
        player_amount = 0;
    }
    raised_ = 0;
}

void Pot::remove_player(int player_id) {
    auto it = player_amounts_.find(player_id);
    if (it != player_amounts_.end()) {
        amount_ += it->second;
        player_amounts_.erase(it);
    }
}

int Pot::get_total_amount() const {
    return amount_ + std::accumulate(player_amounts_.begin(), player_amounts_.end(), 0,
        [](int sum, const auto& pair) { return sum + pair.second; });
}

std::unique_ptr<Pot> Pot::split_pot(int raised_level) const {
    if (raised_ <= raised_level) {
        return nullptr;
    }

    auto split_pot = std::make_unique<Pot>();
    split_pot->raised_ = raised_ - raised_level;

    for (const auto& [player_id, amount] : player_amounts_) {
        if (amount > raised_level) {
            int overflow = amount - raised_level;
            split_pot->player_amounts_[player_id] = overflow;
        }
    }

    return split_pot;
}

