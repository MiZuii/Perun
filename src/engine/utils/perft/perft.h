#pragma once

#include "../common/includes.h"
#include "../common/types.h"
#include "../common/bit_opers.h"

#include "../../board/board_repr.h"

/* Perft - function to traverse all nodes in a given position till given depth and print the result */

void perft(FEN_t fen, int depth);
