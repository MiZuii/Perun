#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"

// const int tab64[64] = {
//     63,  0, 58,  1, 59, 47, 53,  2,
//     60, 39, 48, 27, 54, 33, 42,  3,
//     61, 51, 37, 40, 49, 18, 28, 20,
//     55, 30, 34, 11, 43, 14, 22,  4,
//     62, 57, 46, 52, 38, 26, 32, 41,
//     50, 36, 17, 19, 29, 10, 13, 21,
//     56, 45, 25, 31, 35, 16,  9, 12,
//     44, 24, 15,  8, 23,  7,  6,  5};

// int log2_64 (uint64_t value)
// {
//     value |= value >> 1;
//     value |= value >> 2;
//     value |= value >> 4;
//     value |= value >> 8;
//     value |= value >> 16;
//     value |= value >> 32;
//     return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
// }

/* -------------------------------------------------------------------------- */
/*                                  BITBOARDS                                 */
/* -------------------------------------------------------------------------- */

class BitBoardBase
{
public:
    U64 _bit_board;
    virtual ~BitBoardBase() = 0;

#if DEBUG
    virtual std::string toString() const;
#endif

};


class PieceBitBoard : private BitBoardBase
{
private:
    Piece_t _piece;

public:
    PieceBitBoard(U64 board, Piece_t piece);
    ~PieceBitBoard() = default;

#if DEBUG
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const PieceBitBoard& obj);
#endif
};


class PureBitBoard : private BitBoardBase
{
public:
    PureBitBoard(U64 board);
    ~PureBitBoard() = default;

#if DEBUG
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const PureBitBoard& obj);
#endif
};


/* -------------------------------------------------------------------------- */
/*                                    BOARD                                   */
/* -------------------------------------------------------------------------- */


class Board
{
private:
    /* data */
public:
    Board(/* args */);
    ~Board();
};
