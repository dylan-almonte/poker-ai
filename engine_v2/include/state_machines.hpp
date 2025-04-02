#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include <functional>

class HandPhaseStateMachine {
    private:
        HandPhase::Phase current_phase_;
        std::function<void(HandPhase::Phase)> on_transition_;
        
};

class BettingRound {
    private:
        std::vector<std::shared_ptr<Player>> players_;
        std::vector<std::shared_ptr<Pot>> pots_;
        int starting_player_;
        int last_raise_;
        
        void _move_to_next_player() {
            do {
                starting_player_ = (starting_player_ + 1) % players_.size();
            } while (!players_[starting_player_]->isActive());
        }
        
        void _post_player_bets(int player_idx, int amount) {

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
                auto active_players = _get_active_players();
                auto last_pot = pots_[last_pot_idx];
                for (int pot_player_id : active_players) {
                    if (last_pot->get_player_amount(pot_player_id) > new_raise_level) {
                        new_raise_level = std::min(new_raise_level, last_pot->get_player_amount(pot_player_id));
                    }
                }
                _split_pot(last_pot_idx, new_raise_level);
            }   
            players_[player_idx]->setChips(players_[player_idx]->getChips() - original_amount);
        }

        void _split_pot(int pot_idx, int raise_level) {
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

            for (int player_id = 0; player_id < players_.size(); player_id++) {
                if (players_[player_id]->isActive() && players_[player_id]->getChips() > chipsToCall(player_id)) {
                    players_[player_id]->setLastPot(pot_idx + 1);
                }
            }
        }

        std::vector<int> _get_active_players() {
            std::vector<int> active_players;
            for (int i = 0; i < players_.size(); i++) {
                if (players_[i]->isActive()) {
                    active_players.push_back(i);
                }
            }
            return active_players;
        }

    public:
        BettingRound(std::vector<std::shared_ptr<Player>> players, std::vector<std::shared_ptr<Pot>> pots, int starting_player ) 
                    : players_(players), pots_(pots), starting_player_(starting_player) {}
        void handleAction(Action action);
        void handleRoundComplete(bool last_round, bool all_in);
        void handleHandOver();

        int chipsToCall(int player_id) {
            int to_call = 0;
            for (int i = 0; i < players_[player_id]->getLastPot() + 1; i++) {
                to_call += pots_[i]->chips_to_call(player_id);
            }
            return to_call;
        }

};

    