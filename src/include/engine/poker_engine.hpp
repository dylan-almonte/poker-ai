#pragma once
#include <memory>
#include <vector>
#include <optional>
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "action.hpp"
#include "hand_phase.hpp"

/**
 * @brief Represents the immutable state of a poker game.
 * 
 * The GameState struct contains all information about the current state of the game,
 * including player states, chips, bets, pots, and community cards. All modifications
 * to the state return a new state instance, ensuring immutability.
 */
struct GameState {
    bool is_terminal = false;                    ///< Whether the current hand is complete
    size_t num_players{};                        ///< Number of players in the game
    size_t num_pots{};                           ///< Number of active pots
    size_t current_player{};                     ///< ID of the current player to act
    HandPhase::Phase street{};                   ///< Current betting street (preflop, flop, etc.)
    std::vector<PlayerState> player_states{};    ///< State of each player (IN, OUT, ALL_IN)
    std::vector<int> player_chips{};             ///< Chip count for each player
    std::vector<int> player_bets{};              ///< Current bet amount for each player
    std::vector<int> player_rewards{};           ///< Final rewards for each player
    std::vector<Pot> pots{};                     ///< Active pots in the game
    std::vector<Card> board{ 5, Card(-1) };      ///< Community cards on the board
    std::pair<Card, Card> hole_cards{ Card(-1), Card(-1) };  ///< Current player's hole cards
    HandPhase::Phase hand_phase{ HandPhase::Phase::PREFLOP }; ///< Current phase of the hand

    /**
     * @brief Creates a new state with an updated player state.
     * 
     * @param player The ID of the player to update
     * @param new_state The new state for the player
     * @return A new GameState with the updated player state
     */
    GameState with_player_state(size_t player, PlayerState new_state) const;

    /**
     * @brief Creates a new state with updated player chips.
     * 
     * @param player The ID of the player to update
     * @param new_chips The new chip count for the player
     * @return A new GameState with the updated player chips
     */
    GameState with_player_chips(size_t player, int new_chips) const;

    /**
     * @brief Creates a new state with updated player bets.
     * 
     * @param player The ID of the player to update
     * @param new_bets The new bet amount for the player
     * @return A new GameState with the updated player bets
     */
    GameState with_player_bets(size_t player, int new_bets) const;

    /**
     * @brief Creates a new state with updated player rewards.
     * 
     * @param player The ID of the player to update
     * @param new_rewards The new reward amount for the player
     * @return A new GameState with the updated player rewards
     */
    GameState with_player_rewards(size_t player, int new_rewards) const;

    /**
     * @brief Creates a new state with an additional pot.
     * 
     * @param new_pot The new pot to add
     * @return A new GameState with the additional pot
     */
    GameState with_pot(Pot new_pot) const;

    /**
     * @brief Creates a new state with an updated board card.
     * 
     * @param index The index of the board card to update
     * @param card The new card to place on the board
     * @return A new GameState with the updated board card
     */
    GameState with_board_card(size_t index, Card card) const;

    /**
     * @brief Creates a new state with updated hole cards.
     * 
     * @param new_cards The new hole cards
     * @return A new GameState with the updated hole cards
     */
    GameState with_hole_cards(std::pair<Card, Card> new_cards) const;

    /**
     * @brief Creates a new state with an updated street.
     * 
     * @param new_street The new betting street
     * @return A new GameState with the updated street
     */
    GameState with_street(HandPhase::Phase new_street) const;

    /**
     * @brief Creates a new state with an updated current player.
     * 
     * @param new_player The ID of the new current player
     * @return A new GameState with the updated current player
     */
    GameState with_current_player(size_t new_player) const;

    /**
     * @brief Creates a new state with an updated terminal status.
     * 
     * @param terminal Whether the hand is complete
     * @return A new GameState with the updated terminal status
     */
    GameState with_terminal(bool terminal) const;
};

/**
 * @brief A lightweight, immutable poker game engine.
 * 
 * The PokerEngine class manages the game flow and state transitions in a poker game.
 * It provides methods for starting new hands, processing actions, and determining
 * legal moves. The engine maintains immutability by returning new states for all
 * operations.
 */
class PokerEngine {
private:
    const int small_blind_;      ///< Small blind amount
    const int big_blind_;        ///< Big blind amount
    const size_t num_players_;   ///< Number of players in the game
    Deck deck_;                  ///< Deck of cards for the game

    // Helper methods
    static bool is_betting_round_complete(const GameState& state);
    static bool is_hand_complete(const GameState& state);
    static size_t next_active_player(const GameState& state, size_t start);
    static size_t last_to_act(const GameState& state);
    static int chips_to_call(const GameState& state, size_t player);
    static bool is_valid_action(const GameState& state, const Action& action);

    // State transition methods
    static GameState start_new_hand(const GameState& state, int small_blind, int big_blind);
    static GameState post_blinds(const GameState& state, int small_blind, int big_blind);
    static GameState deal_hole_cards(const GameState& state, Deck& deck);
    static GameState deal_community_cards(const GameState& state, Deck& deck);
    static GameState process_action(const GameState& state, const Action& action);
    static GameState collect_bets(const GameState& state);
    static GameState split_pot(const GameState& state);
    static GameState award_pots(const GameState& state);

public:
    /**
     * @brief Constructs a new poker engine.
     * 
     * @param small_blind The small blind amount
     * @param big_blind The big blind amount
     * @param num_players The number of players in the game
     */
    PokerEngine(int small_blind, int big_blind, size_t num_players);

    /**
     * @brief Creates the initial state for a new hand.
     * 
     * @return A new GameState representing the start of a hand
     */
    GameState initial_state() const;

    /**
     * @brief Gets all legal actions for the current state.
     * 
     * @param state The current game state
     * @return A vector of legal actions
     */
    std::vector<Action> legal_actions(const GameState& state) const;

    /**
     * @brief Processes an action and returns the new game state.
     * 
     * @param state The current game state
     * @param action The action to process
     * @return A new GameState after processing the action
     * @throws std::runtime_error if the state is terminal
     * @throws std::invalid_argument if the action is invalid
     */
    GameState step(const GameState& state, const Action& action) const;

    /**
     * @brief Gets the ID of the current player to act.
     * 
     * @param state The current game state
     * @return The ID of the current player
     */
    size_t current_player(const GameState& state) const;

    /**
     * @brief Checks if the current state is terminal.
     * 
     * @param state The current game state
     * @return true if the hand is complete, false otherwise
     */
    bool is_terminal(const GameState& state) const;

    /**
     * @brief Gets the final rewards for all players.
     * 
     * @param state The current game state
     * @return A vector of rewards for each player
     */
    std::vector<int> rewards(const GameState& state) const;
}; 