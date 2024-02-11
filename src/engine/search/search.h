#pragma once

#include "../board/board_repr.h"

/* -------------------------------------------------------------------------- */
/*                              SEARCH STRUCTURES                             */
/* -------------------------------------------------------------------------- */

struct EngineResults
{
    // calculation state variables
    bool new_data = false;
    bool finished = false;

    int current_depth;
    Move_t best_move;
};

struct SearchArgs
{
    SearchLimitType search_type = DEPTH_LIM;
    int depth_lim = SEARCH_INF;
    int time_lim;
    int node_lim;
};

struct MoveStack
{
    std::vector<Move_t> moves;
};

struct RootMove
{

    Move_t root_move;
    ScoreVal_t score;

    MoveStack mstack;
    ScoreVal_t previous_score;

};


/* -------------------------------------------------------------------------- */
/*                              SEARCH ALGORITHM                              */
/* -------------------------------------------------------------------------- */

void search(std::stop_token stok, Board board, std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx);
void _search(std::stop_token stok, Board board, RootMove &rm, SearchArgs args);
int negamax_ab(Board board, int alpha, int beta, int depth_left, std::stop_token &stok);
int quiesce(Board &board, int alpha, int beta);
