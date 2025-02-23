#include "pot.hpp"
#include <numeric>

int Pot::chips_to_call(int player_id) const {
    auto it = player_amounts.find(player_id);
    return raised - (it != player_amounts.end() ? it->second : 0);
}

void Pot::player_post(int player_id, int amount) {
    player_amounts[player_id] = player_amounts[player_id] + amount;
    if (player_amounts[player_id] > raised) {
        raised = player_amounts[player_id];
    }
}

int Pot::get_player_amount(int player_id) const {
    auto it = player_amounts.find(player_id);
    return it != player_amounts.end() ? it->second : 0;
}

std::vector<int> Pot::players_in_pot() const {
    std::vector<int> players;
    for (const auto& [player_id, _] : player_amounts) {
        players.push_back(player_id);
    }
    return players;
}

void Pot::collect_bets() {
    for (auto& [player_id, player_amount] : player_amounts) {
        amount += player_amount;
        player_amount = 0;
    }
    raised = 0;
}

void Pot::remove_player(int player_id) {
    auto it = player_amounts.find(player_id);
    if (it != player_amounts.end()) {
        amount += it->second;
        player_amounts.erase(it);
    }
}

int Pot::get_total_amount() const {
    return amount + std::accumulate(player_amounts.begin(), player_amounts.end(), 0,
        [](int sum, const auto& pair) { return sum + pair.second; });
} 