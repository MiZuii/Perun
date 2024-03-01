#pragma once

#include "../board/board_repr.h"
#include "ordering.h"
#include "../tt/tt.h"

#include <type_traits>

/* -------------------------------------------------------------------------- */
/*                              SEARCH STRUCTURES                             */
/* -------------------------------------------------------------------------- */

class dthread {
private:
    std::thread _thread;

public:

    dthread() noexcept {}

    template<typename Callable, typename... Args>
    explicit dthread(Callable&& func, Args&&... args) noexcept :
        _thread(std::forward<Callable>(func), std::forward<Args>(args)...)
    {}

    dthread(dthread &other) = delete;
    dthread(dthread &&other) noexcept {
        _thread.swap(other._thread);
    }

    dthread& operator=(dthread& other) = delete;
    dthread& operator=(dthread&& other) noexcept {
        _thread.swap(other._thread);
        return *this;
    }

    ~dthread() noexcept {
        if (_thread.joinable()) {
            _thread.join();
        }
    }
};

struct EngineResults
{
    // calculation state variables
    std::condition_variable new_data;
    bool finished, new_pv;

    // results
    std::vector<Move_t> pv;
    ScoreVal_t          pv_score;
    Depth_t             generation;

    // refreshable variables
    std::chrono::time_point<std::chrono::steady_clock>  tstart;
    unsigned int nc;
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
    unsigned int           _last_todo_idx;

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
    /* nc */
    static inline unsigned int nc = 0;

    /* Search intel */
    static inline std::vector<Move_t> *pv            = nullptr;
    static inline Depth_t              pv_init_depth = 0;
    static inline Depth_t              generation    = 0;

    /* Search controll */
    static inline bool over = false;

    static inline std::mutex              mutex;
    static inline std::condition_variable idle;
    static inline std::vector<SplitPoint> split_points;

    /* Search local */
    static constexpr Depth_t    MAX_PLY = 64;
    static constexpr int        KMS_NUM = 2;
    int                         latest_km_index;
    using KMT = std::array<Move_t, MAX_PLY>;
    std::array<KMT, KMS_NUM>    killer_moves;
    unsigned int                lnc;

    /* condition variable for thread waiting
    if becomes and owner of splitpoint*/
    std::condition_variable pidle;

    /* Threads creation */
    void work();
    void iterate(Board board, SearchArgs args, EngineResults &engr, std::mutex &engmtx);

public:

    static void create_worker();
    static void create_main(Board board, SearchArgs args, EngineResults &engr, std::mutex &engmtx);
    static void signal_end();
    static bool finished() {return over;}
    static unsigned int probe_nc();
    static void clear_statics();

    ScoreVal_t negamax(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left);
    ScoreVal_t quiesce(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left);

    friend class MoveOrder;
    friend class SplitPoint;
};

/* -------------------------------------------------------------------------- */
/*                              SEARCH ALGORITHM                              */
/* -------------------------------------------------------------------------- */

void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx);

/* ----------------------------- HELP FUNCTIONS ----------------------------- */

int movetime_deduction(const SearchArgs args, const Side player);
void update_engres(std::vector<Move_t> &pv, ScoreVal_t score, Depth_t gen, EngineResults &engr, std::mutex &engmtx);
