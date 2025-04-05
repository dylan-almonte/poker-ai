#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "game_state.hpp"
#include <functional>
#include <deque>
#include <unordered_map>

#ifdef DEBUG
#define DEBUG_PRINT(x) std::cout << x << std::endl;
#define VALIDATE_ACTION(action) if (!_valid_action(action)) { throw std::invalid_argument("Invalid action"); }
#else
#define DEBUG_PRINT(x)
#define VALIDATE_ACTION(action)
#endif




class BettingRound {
    private:
        std::vector<std::shared_ptr<Player>> players_;
        std::deque<std::shared_ptr<Player>> active_players_;
        std::vector<std::shared_ptr<Pot>> pots_;
        int starting_player_;
        int current_player_;
        int last_raiser_;
        int last_raise_ = 0;
        
        void _move_to_next_player() {
            do {
                current_player_ = (current_player_ + 1) % players_.size();
            } while (!players_[current_player_]->isActive());
        }
        
        void _post_player_bets(int player_idx, Action action) {
            auto player = players_[player_idx];
            
            int amount = std::min(action.getAmount(), player->getChips());
            int original_amount = amount;
            int last_pot_idx = player->getLastPot();

            // if a player posts, they are in the pot
            if (amount == player->getChips()) {
                player->setState(PlayerState::ALL_IN);
            } else {
                player->setState(PlayerState::IN);
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

        bool _valid_action(Action action) {
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
        
        void _handle_action(Action action) {
            if (!_valid_action(action)) {
                throw std::invalid_argument("Invalid action");
            }
            auto player = players_[current_player_];
            auto current_pot = pots_.back();
            action = _translate_all_in(action);


            switch (action.getActionType()) {
            case ActionType::FOLD:
                active_players_.erase(std::find(active_players_.begin(), active_players_.end(), player));
                player->setState(PlayerState::OUT);
                break;
            case ActionType::CHECK:
                player->setState(PlayerState::IN);
                break;
            case ActionType::CALL:
                int to_call = current_pot->chips_to_call(current_player_);
                player->setChips(player->getChips() - to_call);
                _post_player_bets(current_player_, {ActionType::CALL, to_call}); 
                player->setState(PlayerState::IN);
                break;
            case ActionType::RAISE:
                player->setChips(player->getChips() - action.getAmount());
                _post_player_bets(current_player_, action);
                
                player->setState(PlayerState::IN);
                break;
            case ActionType::ALL_IN:
                int chips = player->getChips();
                player->setChips(0);
                _post_player_bets(current_player_, {ActionType::ALL_IN, chips});
                player->setState(PlayerState::ALL_IN);
                break;
            }
        }

        Action _translate_all_in(Action action) {
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

    public:
        BettingRound(std::vector<std::shared_ptr<Player>> players, std::vector<std::shared_ptr<Pot>> pots, int starting_player, int current_player) 
                    : players_(players), pots_(pots), starting_player_(starting_player), current_player_(current_player) 
            {
                size_t num_players = players_.size();
                for (size_t i = 0; i < num_players; i++) {
                    auto player = players_[(i + starting_player_) % num_players];
                    if (player->isActive()) {
                        active_players_.emplace_back(player);
                    }
                }
            }

        bool handleAction(Action action) {
            VALIDATE_ACTION(action);
            _handle_action(action);
            _move_to_next_player();
            
            return true;
        }

        int chipsToCall(int player_id) {
            int to_call = 0;
            for (int i = 0; i < players_[player_id]->getLastPot() + 1; i++) {
                to_call += pots_[i]->chips_to_call(player_id);
            }
            return to_call;
        }

};

    