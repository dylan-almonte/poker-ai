#include "hand_phase.hpp"
#include <string>

std::string phaseToString(HandPhase::Phase phase) {
    switch (phase) {
        case HandPhase::Phase::PREHAND: return "PREHAND";
        case HandPhase::Phase::PREFLOP: return "PREFLOP";
        case HandPhase::Phase::FLOP: return "FLOP";
        case HandPhase::Phase::TURN: return "TURN";
        case HandPhase::Phase::RIVER: return "RIVER";
        case HandPhase::Phase::SETTLE: return "SETTLE";
        default: return "UNKNOWN";
    }
} 