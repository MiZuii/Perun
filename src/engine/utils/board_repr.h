#pragma once

#include <inttypes.h>
#include <string>
#include "piece_types.h"

class BitBoard
{
private:
    uint64_t _bit_board;
    Piece_t _piece;
public:
    BitBoard(uint64_t board, Piece_t piece);
    ~BitBoard() = default;
    operator std::string() const;
};


class Board
{
private:
    /* data */
public:
    Board(/* args */);
    ~Board();
};
