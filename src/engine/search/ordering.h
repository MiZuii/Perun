#pragma once

#include "../board/board_repr.h"

// forward declaration of RootMove struct
struct RootMove;

constexpr int mvv_lva[12][13] = {
    {0  , 0  , 0  , 0  , 0  , 0  , 0  , 501, 401, 301, 201, 101, 0},
    {0  , 0  , 0  , 0  , 0  , 0  , 0  , 502, 402, 302, 202, 102, 0},
    {0  , 0  , 0  , 0  , 0  , 0  , 0  , 503, 403, 303, 203, 103, 0},
    {0  , 0  , 0  , 0  , 0  , 0  , 0  , 504, 404, 304, 204, 104, 0},
    {0  , 0  , 0  , 0  , 0  , 0  , 0  , 505, 405, 305, 205, 105, 0},
    {0  , 0  , 0  , 0  , 0  , 0  , 0  , 506, 406, 306, 206, 106, 0},
    {0  , 501, 401, 301, 201, 101, 0  , 0  , 0  , 0  , 0  , 0  , 0},
    {0  , 502, 402, 302, 202, 102, 0  , 0  , 0  , 0  , 0  , 0  , 0},
    {0  , 503, 403, 303, 203, 103, 0  , 0  , 0  , 0  , 0  , 0  , 0},
    {0  , 504, 404, 304, 204, 104, 0  , 0  , 0  , 0  , 0  , 0  , 0},
    {0  , 505, 405, 305, 205, 105, 0  , 0  , 0  , 0  , 0  , 0  , 0},
    {0  , 506, 406, 306, 206, 106, 0  , 0  , 0  , 0  , 0  , 0  , 0}
};  //usage: [Attacking piece][Victim piece] (kings cant be captured -> 0)
    //the 13 column is no_piece (saves one if on the lookup)

class MoveOrder
{
private:
    Board &_board;
    RootMove &_rm;
    int _d;
    std::unordered_map<Move_t, int> _mvalmap;

public:

    /* -------------------------------- IMPORTANT ------------------------------- */
    /* The class in constructor creates a vector of target pieces to not calculate
    them while sorting. Because of that for proper sort the board passed as argument
    to constructor must already have generated moves on which the class will create
    target piece lookup(a hashmap) */

    MoveOrder(Board &board, RootMove &rm, int d);

    // operator for std::sort
    bool operator() (const Move_t& a, const Move_t& b);
};
