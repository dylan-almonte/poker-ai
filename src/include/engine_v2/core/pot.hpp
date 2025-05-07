#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

/**
 * @brief Represents a betting pot in a poker game.
 * 
 * A pot tracks the amount of chips in the pot, the highest bet amount,
 * and how much each player has contributed to the current betting round.
 */
class Pot {
private:
    int amount_;                                    // Amount of chips in the pot from previous rounds
    int raised_;                                    // Highest bet amount in the current betting round
    std::unordered_map<int, int> player_amounts_;   // Map from player_id to amount bet this round

public:
    /**
     * @brief Default constructor initializes an empty pot.
     */
    Pot() : amount_(0), raised_(0) {}

    /**
     * @brief Copy constructor.
     */
    Pot(const Pot& other) = default;

    /**
     * @brief Move constructor.
     */
    Pot(Pot&& other) noexcept = default;

    /**
     * @brief Assignment operator.
     */
    Pot& operator=(const Pot& other) = default;

    /**
     * @brief Move assignment operator.
     */
    Pot& operator=(Pot&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~Pot() = default;

    /**
     * @brief Returns the amount of chips a player needs to call.
     * 
     * @param player_id The ID of the player
     * @return The amount the player needs to call
     */
    int chips_to_call(int player_id) const;

    /**
     * @brief Posts chips from a player into the pot.
     * 
     * @param player_id The ID of the player posting chips
     * @param amount The amount of chips to post
     */
    void player_post(int player_id, int amount);

    /**
     * @brief Creates a new pot with the player's chips posted.
     * 
     * This is an immutable version of player_post that returns a new pot.
     * 
     * @param player_id The ID of the player posting chips
     * @param amount The amount of chips to post
     * @return A new pot with the updated state
     */
    Pot with_player_post(int player_id, int amount) const;

    /**
     * @brief Gets the amount a player has bet in the current round.
     * 
     * @param player_id The ID of the player
     * @return The amount the player has bet
     */
    int get_player_amount(int player_id) const;

    /**
     * @brief Gets the amount a player has bet in the current round.
     * 
     * @param player_id The ID of the player
     * @return Optional containing the amount if the player has bet, nullopt otherwise
     */
    std::optional<int> get_player_amount_optional(int player_id) const;

    /**
     * @brief Gets a list of all players who have chips in the pot.
     * 
     * @return Vector of player IDs
     */
    std::vector<int> players_in_pot() const;

    /**
     * @brief Applies a function to each player in the pot.
     * 
     * @param callback Function to apply to each player ID
     */
    template<typename F>
    void for_each_player_in_pot(F&& callback) const {
        for (const auto& [player_id, _] : player_amounts_) {
            callback(player_id);
        }
    }

    /**
     * @brief Collects all bets from the current round into the pot amount.
     * 
     * This resets the player amounts and raised amount for the next round.
     */
    void collect_bets();

    /**
     * @brief Removes a player from the pot and adds their chips to the pot amount.
     * 
     * @param player_id The ID of the player to remove
     */
    void remove_player(int player_id);

    /**
     * @brief Gets the amount of chips in the pot from previous rounds.
     * 
     * @return The amount of chips in the pot
     */
    int get_amount() const { return amount_; }

    /**
     * @brief Gets the highest bet amount in the current round.
     * 
     * @return The highest bet amount
     */
    int get_raised() const { return raised_; }

    /**
     * @brief Gets the total amount of chips in the pot including current bets.
     * 
     * @return The total amount of chips in the pot
     */
    int get_total_amount() const;

    /**
     * @brief Splits the pot at the given raised level.
     * 
     * Creates a new pot with the overflow from players who bet more than the raised level.
     * 
     * @param raised_level The chip count to cut off at
     * @return A new pot with the overflow, or nullptr if no split is needed
     */
    std::unique_ptr<Pot> split_pot(int raised_level) const;
}; 