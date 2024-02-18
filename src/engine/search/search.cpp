#include "search.h"

#include "../interface/interface.h"

void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    // init
    board.getMoves();
    std::vector<std::jthread>   root_threads;
    std::atomic<int>            completion_counter          = 0;
    const unsigned int          number_of_search_threads    = board.moves.size();
    const Side                  player                      = board.sideToMove();
    const auto                  loc_start                   = std::chrono::steady_clock::now();

    // search args init
    if( TIME_LIM == args.search_type && -1 == args.time_lim)
    {
        args.time_lim = movetime_deduction(args, player);
    }

    // start time ocunt (after this line till creation of search threads there should be nothing)
    {
        std::lock_guard<std::mutex> end_guard(engmtx);
        engr.tstart = std::chrono::steady_clock::now();
    }

    // init searching threads
    for(Move_t move : board.moves)
    {
        root_threads.push_back(std::jthread(_search, Board(board, move), 
            move, std::ref(engr) , std::ref(engmtx), args, player, std::ref(completion_counter),
            std::ref(move_hist)));
    }

    // controll loop
    while(!stok.stop_requested())
    {
        if(number_of_search_threads <= completion_counter.load())
        {
            break;
        }

        if( std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::steady_clock::now() - loc_start).count() >= args.time_lim &&
            args.search_type == TIME_LIM)
        {
            break;
        }

        std::this_thread::sleep_for(0ms);
    }

    // exiting must dos'
    {
        std::lock_guard<std::mutex> end_guard(engmtx);
        engr.finished = true;
    }

    Engine::engine_running = false;
}

void _search(std::stop_token stok, Board board, Move_t move,
    EngineResults &engr, std::mutex &engmtx, SearchArgs args, 
    const Side player, std::atomic<int> &comp_counter, 
    const std::vector<Move_t> &move_hist)
{
    // initiate search type and parameters based on the RootMove and SearchArgs information
    const int loc_depth_lim     = args.search_type == DEPTH_LIM ? args.depth_lim : SEARCH_INF;
    const int loc_depth_start   = args.depth_start;
    RootMove  rm                = {move, 0, 0, 
                                   {std::vector<Move_t>(move_hist), move_hist.size() + 1}, 
                                   0, player, stok};

    rm.mstack.moves.push_back(move);

    // search
    for(int d=loc_depth_start; d < loc_depth_lim; d++)
    {
        if(stok.stop_requested())
        {
            break;
        }

        ScoreVal_t lscore   = -negamax_ab(board, -EVAL_INF, EVAL_INF, d, rm);
        rm.score            = lscore;
        rm.eval_depth       = d;

        update_engres(rm, engr, engmtx);
    }

    comp_counter++;
}

int negamax_ab(Board board, int alpha, int beta, int depth_left, RootMove &rm)
{
    if(rm.stok.stop_requested())
    {
        return 0;
    }

    // search lim check
    if( 0 == depth_left )
    {
        return quiesce(board, alpha, beta, rm);
    }

    // generate moves for current node
    board.getMoves();

    if(board.moves.empty())
    {
        return board.getCheckers() ? -EVAL_INF : 0;
    }

    for(Move_t move : board.moves)
    {
        rm.mstack.moves.push_back(move);
        ScoreVal_t local_score = -negamax_ab({board, move}, -beta, -alpha, depth_left-1, rm);
        rm.mstack.moves.pop_back();

        if( local_score >= beta)
        {
            return beta; // cutof (hard/soft base on the current negamax node)
        }
        if( local_score > alpha )
        {
            alpha = local_score;
        }
    }
    return alpha;
}

int quiesce(Board board, int alpha, int beta, RootMove &rm)
{
    // generate moves for current node
    board.getMoves();

    if(board.moves.empty())
    {
        return board.getCheckers() ? -EVAL_INF : 0;
    }

    rm.nc++;
    int stand_pat = board.sideToMove() == WHITE ? -evaluate<true>(board) : evaluate<false>(board);

    if( stand_pat >= beta )
    {
        return beta;
    }
    if( alpha < stand_pat )
    {
        alpha = stand_pat;
    }

    for(Move_t move : board.moves)
    {
        if(!getCaptureFlag(move))
        {
            continue;
        }

        rm.mstack.moves.push_back(move);
        ScoreVal_t local_score = -quiesce({board, move}, -beta, -alpha, rm);
        rm.mstack.moves.pop_back();

        if( local_score >= beta )
        {
            return beta;
        }
        if( alpha < local_score )
        {
           alpha = local_score;
        }
    }
    return alpha;
}

int movetime_deduction(const SearchArgs args, const Side player)
{
    // returns time in milliseconds
    return 5000;
}

void update_engres(RootMove &rm, EngineResults &engr, std::mutex &engmtx)
{
    std::lock_guard<std::mutex> end_guard(engmtx);

    bool new_data_flag = false;

    // score update
    if((rm.player == WHITE ? rm.score > engr.best_score : rm.score > -engr.best_score) || engr.best_move == 0)
    {
        engr.best_move  = rm.root_move;
        engr.best_score = rm.player == WHITE ? rm.score : -rm.score;
        new_data_flag   = true;
    }

    // depth update
    if(rm.eval_depth > engr.current_max_depth)
    {
        engr.current_max_depth  = rm.eval_depth;
        new_data_flag           = true;
    }

    if(rm.nc != 0)
    {
        engr.node_count.fetch_add(rm.nc);
        rm.nc = 0;
        new_data_flag = true;
    }

    // wakeup if new data 
    if(new_data_flag)
    {
        engr.new_data.notify_all();
    }
}
