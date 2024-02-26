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
    int wtime = -1;
    int btime = -1;
    int winc = -1;
    int binc = -1;
    int movestogo = -1;
};

enum WorkState
{
    TODO,       // move is yet to be calculated
    WORKING,    // move is beeing calculated
    WAITING,    // move is yet to be merged to splitpoint
    DONE        // move was correctly merged
};

struct MoveState
{
    Move_t      move;
    ScoreVal_t  score;
    int         state;
};

class SplitPoint
{
private:

    SearchThread           &_owner;
    MoveState              _main_move_state;
    std::vector<MoveState> _moves;
    int                    _last_todo_idx;

public:

    SplitPoint(SearchThread &owner, Board &board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left);

    bool       finished();
    MoveState *get_work();
    void       work_done(MoveState *ms, ScoreVal_t res);

    // state
    Board      &board;
    ScoreVal_t alpha, beta;
    Depth_t    depth_left;

    friend class SearchThread;
};


class SearchThread
{
private:
    /* Search intel */
    static std::vector<Move_t> *pv;
    static Depth_t              pv_init_depth;
    static Depth_t              generation;

    /* Search controll */
    static bool over;

    static std::mutex              mutex;
    static std::condition_variable idle;
    static std::vector<SplitPoint> split_points;

    /* Search local */
    static constexpr Depth_t    MAX_PLY = 64;
    static constexpr int        KMS_NUM = 2;
    int                         latest_km_index;
    using KMT = std::array<Move_t, MAX_PLY>;
    std::array<KMT, KMS_NUM>    killer_moves;
    int                         nc;

    /* condition variable for thread waiting
    if becomes and owner of splitpoint*/
    std::condition_variable pidle;


public:

    static void create_worker();
    static void create_main();

    ScoreVal_t negamax(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left);
    ScoreVal_t quiesce(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left);

    friend class MoveOrder;
    friend class SplitPoint;

    void work();
    void iterate();
};

/* -------------------------------------------------------------------------- */
/*                              SEARCH ALGORITHM                              */
/* -------------------------------------------------------------------------- */

void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx);

/* ----------------------------- HELP FUNCTIONS ----------------------------- */

int movetime_deduction(const SearchArgs args, const Side player);
void update_engres(SearchThread &rm, EngineResults &engr, std::mutex &engmtx, bool final = false);
