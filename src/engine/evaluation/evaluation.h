#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "../board/board_repr.h"

enum MaterialScores
{

    PAWN_SCORE = 100,
    KNIGHT_SCORE = 300,
    BISHOP_SCORE = 300,
    ROOK_SCORE = 500,
    QUEEN_SCORE = 900

};

template<bool WhiteMove>
int evaluate(Board &board);
