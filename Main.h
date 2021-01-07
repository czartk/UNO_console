#ifndef MAIN_H
#define MAIN_H

#include <random>
#include <ctime>

// Namespaces

namespace Mersenne
{
    // Initialize our mersenne twister with a random seed based on the clock (once at system startup)
    inline std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
}

// Enum declarations

enum class Technical
{
    PLAY_COUNTER_TEXT,
    PLAY_TAKEN_TEXT,

    MAX_TECHNICAL
};

enum class AI_PERSONAS  // Max 9 ai players
{
    HUMAN,          // Always first (==0)
    NORMAL,         // Normal player, acts normally (==1) const
    CAUTIOUS,       // Especially cautious
    BOLD,           // Is more daring than others (reasonably)
    UNPREDICTABLE,  // His moves are fully random
    CASUAL,         // Takes some chances, doesn't really plan ahead
    ADVANCED,       // Acts as reasonable as the coder can

    MAX_PERSONAS
};

#endif
