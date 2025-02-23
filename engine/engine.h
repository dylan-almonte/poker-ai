#pragma once
#include "game.hpp"
#include "player.hpp"
#include "card.hpp"
#include "action_type.hpp"
#include "hand_phase.hpp"
#include "player_state.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "evaluator.hpp"

#define POKER_ENGINE_NAMESPACE_BEGIN namespace poker { namespace engine {
#define POKER_ENGINE_NAMESPACE_END } }

POKER_ENGINE_NAMESPACE_BEGIN
    using Game = ::Game;
    using Player = ::Player;
    using Card = ::Card;
    using ActionType = ::ActionType;
    using HandPhase = ::HandPhase;
    using PlayerState = ::PlayerState;
    using Pot = ::Pot;
    using Deck = ::Deck;
    using Evaluator = ::Evaluator;
POKER_ENGINE_NAMESPACE_END