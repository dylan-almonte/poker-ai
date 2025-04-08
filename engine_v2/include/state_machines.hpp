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
#include <iostream>
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
        std::vector<std::shared_ptr<Pot>> pots_;
        std::deque<int> active_players_;
        int current_player_;
        int last_to_act_;
        int all_in_count_ = 0;
        
        void _move_to_next_player() {
            do {
                current_player_ = (current_player_ + 1) % players_.size();
            } while (!players_[current_player_]->isActive());
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

            for (size_t pot_player_id : pot_players) {
                if (pot->get_player_amount(pot_player_id) > raise_level) {
                    split_pot.player_post(pot_player_id, pot->get_player_amount(pot_player_id) - raise_level);
                }
            }

            for (size_t player_id = 0; player_id < players_.size(); player_id++) {
                if (players_[player_id]->isActive() && players_[player_id]->getChips() > chipsToCall(player_id)) {
                    players_[player_id]->setLastPot(pot_idx + 1);
                }
            }
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
            auto player = players_[current_player_];
            auto current_pot = pots_.back();
            action = _translate_all_in(action);

            
            switch (action.getActionType()) {
            case ActionType::FOLD:
                active_players_.erase(std::find(active_players_.begin(), active_players_.end(), current_player_));
                player->setState(PlayerState::OUT);
                break;
            case ActionType::CHECK:
                player->setState(PlayerState::IN);
                break;
            case ActionType::CALL: {
                int to_call = current_pot->chips_to_call(current_player_);
                post_player_bets(current_player_, to_call); 
                break;
            }
    
            case ActionType::RAISE:
                post_player_bets(current_player_, action.getAmount());
                last_to_act_ = current_player_;
                break;
            case ActionType::ALL_IN:
                int chips = player->getChips();
                player->setChips(0);
                post_player_bets(current_player_, chips);
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
        BettingRound(const BettingRound&) = default;  // Copy constructor
        BettingRound& operator=(const BettingRound&) = default;  // Copy assignment
        BettingRound(BettingRound&&) = default;  // Move constructor
        BettingRound& operator=(BettingRound&&) = default;  // Move assignment
        ~BettingRound() = default;  // Destructor
        BettingRound() : players_(), pots_(), last_to_act_(-1) {}
        BettingRound(std::vector<std::shared_ptr<Player>> players, 
                     std::vector<std::shared_ptr<Pot>> pots, 
                     int last_to_act) : players_(players), pots_(pots), last_to_act_(last_to_act) 
            {
                size_t num_players = players_.size();
                current_player_ = (last_to_act_ + 1) % num_players;
                for (size_t i = 0; i < num_players; i++) {
                    auto player = players_[(i + last_to_act_ + 1) % num_players];
                    if (player->isActive()) {
                        active_players_.emplace_back(i);
                    }
                }
                
            }

        void post_player_bets(int player_idx, int amount) {
            auto player = players_[player_idx];
            
            amount = std::min(amount, player->getChips());
            int original_amount = amount;
            int last_pot_idx = player->getLastPot();

            // if a player posts, they are in the pot
            if (amount >= player->getChips()) {
                player->setState(PlayerState::ALL_IN);
                active_players_.erase(std::find(active_players_.begin(), active_players_.end(), player_idx));
                all_in_count_++;
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
                auto last_pot = pots_[last_pot_idx];
                for (int pot_player_id : active_players_) {
                    if (last_pot->get_player_amount(pot_player_id) >= new_raise_level) {
                        new_raise_level = std::min(new_raise_level, last_pot->get_player_amount(pot_player_id));
                    }
                }
                _split_pot(last_pot_idx, new_raise_level);
            }   
            players_[player_idx]->setChips(players_[player_idx]->getChips() - original_amount);
        }

        bool handleAction(Action action) {
            VALIDATE_ACTION(action);
            _handle_action(action);
            if (everyoneAllIn()) {
                return true;
            }
            _move_to_next_player();
            
            return current_player_ == last_to_act_;
        }

        int getCurrentPlayer() {
            return current_player_;
        }
        int everyoneAllIn() {
            return active_players_.size() == 0;
            return all_in_count_ == pots_.back()->players_in_pot().size();
        }

        int chipsToCall(int player_id) {
            int to_call = 0;
            for (int i = 0; i < players_[player_id]->getLastPot() + 1; i++) {
                to_call += pots_[i]->chips_to_call(player_id);
            }
            return to_call;
        }

};

    