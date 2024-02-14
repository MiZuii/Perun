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
        if(number_of_search_threads == completion_counter.load())
        {
            break;
        }
    }

    // exiting must dos'
    engmtx.lock();
    engr.finished = true;
    engmtx.unlock();

    Game::game_running = false;
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

        ScoreVal_t lscore   = negamax_ab(board, -EVAL_INF, EVAL_INF, d, rm);
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

    // generate moves for current node
    board.getMoves();

    if(board.moves.empty())
    {
        if(board.getCheckers())
        {
            return board.sideToMove() == rm.player ? EVAL_INF : -EVAL_INF;
        }
        else
        {
            return 0;
        }
    }

    // search lim check
    if( 0 == depth_left )
    {
        return quiesce(board, alpha, beta, rm);
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

int quiesce(Board &board, int alpha, int beta, RootMove &rm)
{
    // should be moved to eval
    rm.nc++;

    if(board.sideToMove() == WHITE)
    {
        return board.sideToMove() == rm.player ? evaluate<true>(board) : -evaluate<true>(board);
    }
    else
    {
        return board.sideToMove() == rm.player ? evaluate<false>(board) : -evaluate<false>(board);
    }
}

void update_engres(RootMove &rm, EngineResults &engr, std::mutex &engmtx)
{
    engmtx.lock();

    bool new_data_flag = false;

    // score update
    if((rm.player == WHITE ? rm.score > engr.best_score : rm.score < engr.best_score) || engr.best_move == 0)
    {
        engr.best_move  = rm.root_move;
        engr.best_score = rm.score;
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

    // set flag if anything changed
    engr.new_data = new_data_flag;

    engmtx.unlock();
}
