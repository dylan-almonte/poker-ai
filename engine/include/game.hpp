/**
 * @file game.hpp
 * @brief Defines the Game class for managing a poker game session
 * @author dcabahug
 * 
 * This file contains the definition of the Game class, which manages a complete poker game session.
 * It handles game flow, player actions, betting rounds, and hand settlement.
 */

#pragma once
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "game_state.hpp"
#include "betting_round.hpp"
#include "hand_phase.hpp"

/**
 * @class Game
 * @brief Manages a complete poker game session
 * 
 * The Game class is the main controller for a poker game session. It handles:
 * - Game initialization and setup
 * - Hand progression through different phases (pre-flop, flop, turn, river)
 * - Player action processing
 * - Betting round management
 * - Hand settlement and pot distribution
 * - Game state tracking and reporting
 */
class Game {
private:
    std::vector<Action> action_history_;                    ///< History of all actions taken in the current hand
    std::vector<std::shared_ptr<Player>> players_;         ///< All players in the game
    std::vector<std::shared_ptr<Pot>> pots_;              ///< All pots (main and side pots)
    std::vector<Card> board_;                             ///< Community cards on the board
    std::unique_ptr<BettingRound> betting_round_;         ///< Current betting round manager
    Deck deck_;                                           ///< Deck of cards for the game
    HandPhase::Phase phase_;                              ///< Current phase of the hand
    int btn_loc_;                                         ///< Position of the dealer button
    int current_player_;                                  ///< Index of the current player to act
    int small_blind_;                                     ///< Small blind amount
    int big_blind_;                                       ///< Big blind amount
    int last_raise_ = 0;                                  ///< Amount of the last raise
    bool _isHandOver() const;                             ///< Checks if the current hand is over

    /**
     * @brief Processes a player action and updates game state
     * @param action The action to process
     * 
     * Internal method to handle the consequences of player actions
     * and update the game state accordingly.
     */
    void _takeAction(Action action);

    /**
     * @brief Deals cards to players and the board
     * 
     * Initializes a new hand by dealing hole cards to players
     * and community cards to the board according to the current phase.
     */
    void _deal_cards();
    
    /**
     * @brief Splits a pot when an all-in situation occurs
     * @param pot_idx Index of the pot to split
     * @param raise_level The betting level at which to split the pot
     * 
     * Creates side pots when players go all-in with different stack sizes.
     */
    void _split_pot(int pot_idx, int raise_level);
    
    /**
     * @brief Moves the dealer button and blind positions
     * 
     * Updates the positions of the dealer button and blind positions
     * for the next hand.
     */
    void _move_blinds();

    /**
     * @brief Settles the current hand and distributes pots
     * 
     * Determines winners and distributes pots to players based on
     * their hand rankings and betting actions.
     */
    void _settle_hand();

public:
    /**
     * @brief Constructs a new poker game
     * @param num_players Number of players in the game
     * @param starting_chips Starting chip stack for each player
     * @param small_blind Small blind amount
     * @param big_blind Big blind amount
     * 
     * Initializes a new poker game with the specified number of players,
     * starting chips, and blind amounts.
     */
    Game(int num_players, int starting_chips, int small_blind, int big_blind);
    
    /**
     * @brief Starts a new hand
     * @param btn_loc Optional dealer button position (-1 for auto-increment)
     * @return GameState Initial state of the new hand
     * 
     * Initializes a new hand by dealing cards, setting up betting rounds,
     * and returning the initial game state.
     */
    GameState startHand(int btn_loc = -1);

    /**
     * @brief Processes a player action
     * @param action The action to process
     * @return GameState Updated game state after the action
     * 
     * Main entry point for processing player actions. Updates the game state
     * and returns the new state after processing the action.
     */
    GameState takeAction(Action action);

    /**
     * @brief Checks if the current hand is over
     * @return true if the hand is complete, false otherwise
     */
    bool isHandOver() const;

    /**
     * @brief Checks if the game session is complete
     * @return true if the game is complete, false otherwise
     */
    bool isHandComplete() const;
    
    /**
     * @brief Gets the current board cards
     * @return Reference to the vector of community cards
     */
    const std::vector<Card>& getBoard() const { return board_; }

    /**
     * @brief Gets all players in the game
     * @return Reference to the vector of players
     */
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }

    /**
     * @brief Gets all pots in the game
     * @return Reference to the vector of pots
     */
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }

    /**
     * @brief Gets the current player's index
     * @return Index of the current player
     */
    int getCurrentPlayer() const { return current_player_; }

    /**
     * @brief Gets the current hand phase
     * @return Current phase of the hand
     */
    HandPhase::Phase getPhase() const { return phase_; }
    
    /**
     * @brief Prints the current game state to stdout
     * 
     * Debug helper to display the current state of the game,
     * including board cards, player states, and pot information.
     */
    void printState() const;

    /**
     * @brief Gets the current game state
     * @return GameState object containing the current game state
     * 
     * Returns a complete snapshot of the current game state, including:
     * - Player information (chips, bets, states)
     * - Board cards
     * - Current phase
     * - Pot information
     */
    GameState getGameState() const {
        GameState state;
        state.num_players = players_.size();
        state.num_pots = pots_.size();
        state.current_player = current_player_;
        state.street = phase_;
        state.player_chips = std::vector<int>(players_.size(), 0);
        state.player_bets = std::vector<int>(players_.size(), 0);
        state.player_rewards = std::vector<int>(players_.size(), 0);
        state.player_states = std::vector<PlayerState>(players_.size(), PlayerState::OUT);
        for (size_t i = 0; i < players_.size(); ++i) {
            state.player_chips[i] = players_[i]->getChips();
            state.player_bets[i] = pots_.back()->get_player_amount(i);
            state.player_states[i] = players_[i]->getState();
        }
        state.board = std::vector<Card>(5, Card(-1));
        for (size_t i = 0; i < board_.size(); ++i) {
            state.board[i] = board_[i];
        }
        state.hole_cards = std::pair<Card, Card>(players_[current_player_]->getHand()[0], 
                                                 players_[current_player_]->getHand()[1]);
        state.hand_phase = phase_;
        return state;
    }
    // Game state helpers

    // Debug helper
}; 