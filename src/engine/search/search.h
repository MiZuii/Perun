#pragma once

#include "../board/board_repr.h"

/* -------------------------------------------------------------------------- */
/*                              SEARCH STRUCTURES                             */
/* -------------------------------------------------------------------------- */

struct EngineResults
{
    // calculation state variables
    bool        new_data;
    bool        finished;

    Move_t      best_move;
    ScoreVal_t  best_score;
    Depth_t     current_max_depth;

    std::atomic<int> node_count;
};

struct SearchArgs
{
    SearchLimitType search_type = DEPTH_LIM;
    Depth_t         depth_lim   = SEARCH_INF;
    Depth_t         depth_start = 0;
    int             time_lim;
    int             node_lim;
};

struct MoveStack
{
    std::vector<Move_t> moves;
    Depth_t             ms_idx; // start of non history moves (root counts as history)
};

struct RootMove
{
    Move_t      root_move;
    ScoreVal_t  score;
    Depth_t     eval_depth;

    MoveStack   mstack;

    int         nc;
    const Side  player;

    const std::stop_token &stok;
};

/* -------------------------------------------------------------------------- */
/*                              SEARCH ALGORITHM                              */
/* -------------------------------------------------------------------------- */

void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx);
void _search(std::stop_token stok, Board board, Move_t move, EngineResults &engr, 
    std::mutex &engmtx, SearchArgs args, const Side player, std::atomic<int> &comp_counter,
    const std::vector<Move_t> &move_hist);
int negamax_ab(Board board, int alpha, int beta, int depth_left, RootMove &rm);
int quiesce(Board &board, int alpha, int beta, RootMove &rm);

/* ----------------------------- HELP FUNCTIONS ----------------------------- */

void update_engres(RootMove &rm, EngineResults &engr, std::mutex &engmtx);
