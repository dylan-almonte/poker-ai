#pragma once
#include <unordered_map>
#include <vector>

class Pot {
private:
    int amount_;
    int raised_;
    std::unordered_map<int, int> player_amounts_;

public:
    Pot() : amount_(0), raised_(0) {}

    int chips_to_call(int player_id) const;
    void player_post(int player_id, int amount);
    int get_player_amount(int player_id) const;
    std::vector<int> players_in_pot() const;
    void collect_bets();
    void remove_player(int player_id);
    int get_amount() const { return amount_; }
    int get_raised() const { return raised_; }
    int get_total_amount() const;
}; 