#include "pot.hpp"
#include <numeric>

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

int Pot::get_player_amount(int player_id) const {
    auto it = player_amounts_.find(player_id);
    return it != player_amounts_.end() ? it->second : 0;
}

std::vector<int> Pot::players_in_pot() const {
    std::vector<int> players;
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