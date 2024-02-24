#pragma once

#include "../board/board_repr.h"
#include "ordering.h"
#include "../tt/tt.h"

/* -------------------------------------------------------------------------- */
/*                              SEARCH STRUCTURES                             */
/* -------------------------------------------------------------------------- */

struct EngineResults
{
    // calculation state variables
    std::condition_variable new_data;
    bool                    finished;

    // conditional variables
    Move_t      best_move;
    ScoreVal_t  best_score;
    Depth_t     current_max_depth;

    // refreshable variables
    std::atomic<int>                                    node_count;
    std::chrono::time_point<std::chrono::steady_clock>  tstart;
};

struct SearchArgs
{
    SearchLimitType search_type = TIME_LIM;
    Depth_t         depth_lim   = SEARCH_INF;
    Depth_t         depth_start = 1;
    int             time_lim = -1;
    int             node_lim = -1;

    // time args
    int             wtime = -1;
    int             btime = -1;
    int             winc = -1;
    int             binc = -1;
    int             movestogo = -1;
};

struct MoveStack
{
    std::vector<Move_t> moves;
    Depth_t             ms_idx; // start of non history moves (root counts as history)
};

struct RootMove
{
    // basic
    Move_t      root_move;
    ScoreVal_t  score;
    Depth_t     eval_depth;

    // killer and history moves
    static constexpr Depth_t    MAX_PLY = 64;
    static constexpr int        KMS_NUM = 2;
    int                         latest_km_index;
    std::array<std::array<Move_t, MAX_PLY>, KMS_NUM> killer_moves;

    // idk
    MoveStack   mstack;

    // stats
    int         nc;
    const Side  player;

    // termination
    const std::stop_token &stok;
};

/* -------------------------------------------------------------------------- */
/*                              SEARCH ALGORITHM                              */
/* -------------------------------------------------------------------------- */

void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx);
void _search(std::stop_token stok, Board board, Move_t move, EngineResults &engr, 
    std::mutex &engmtx, SearchArgs args, const Side player, std::atomic<unsigned int> &comp_counter,
    const std::vector<Move_t> &move_hist);
int negamax_ab(Board board, int alpha, int beta, int depth_left, RootMove &rm);
int quiesce(Board board, int alpha, int beta, Depth_t depth_left, RootMove &rm);

/* ----------------------------- HELP FUNCTIONS ----------------------------- */

int movetime_deduction(const SearchArgs args, const Side player);
void update_engres(RootMove &rm, EngineResults &engr, std::mutex &engmtx, bool final = false);
