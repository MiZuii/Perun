#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

/* -------------------------------------------------------------------------- */
/*                              SEARCH STRUCTURES                             */
/* -------------------------------------------------------------------------- */

struct MoveStack
{
    std::vector<Move_t> moves;
};

struct RootMove
{

    MoveStack mstack;
    ScoreVal_t score;
    ScoreVal_t previous_score;

};


/* -------------------------------------------------------------------------- */
/*                              SEARCH ALGORITHM                              */
/* -------------------------------------------------------------------------- */

void search();
int negamax_ab(int alpha, int beta, int depth_left);
int quiesce(int alpha, int beta);
