#include "search.h"

#include "../interface/game.h"

void search(std::stop_token stok, Board board, std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    // init
    board.getMoves();
    std::vector<RootMove> root_moves;
    std::vector<std::jthread> root_threads;
    ScoreVal_t bs = 0, previous_bs = 0;

    // start searching
    for(Move_t move : board.moves)
    {
        root_moves.push_back(RootMove(move, 0, MoveStack(), ScoreVal_t()));
        root_threads.push_back(std::jthread(_search, Board(board, move) ,std::ref(root_moves.back()), args));
    }

    RootMove *best_move = &root_moves[0];

    // controll loop
    while(!stok.stop_requested())
    {
        for(RootMove &rm : root_moves)
        {
            if(rm.score > bs)
            {
                bs = rm.score;
                best_move = &rm;
            }
        }

        if(previous_bs != bs)
        {
            engr.best_move = best_move->root_move;
            previous_bs = bs;

            engmtx.lock();
            engr.new_data = true;
            engmtx.unlock();
        }
    }

    // stop searching threads
    for(std::jthread &jt : root_threads)
    {
        jt.request_stop();
    }

    for(std::jthread &jt : root_threads)
    {
        jt.join();
    }

    // exiting must dos'
    engmtx.lock();
    engr.new_data = true;
    engr.finished = true;
    engmtx.unlock();

    Game::game_running = false;
}

void _search(std::stop_token stok, Board board, RootMove &rm, SearchArgs args)
{
    // initiate search type and parameters based on the RootMove and SearchArgs information
    const int loc_depth_lim = SEARCH_INF;
    const int loc_depth_start = 4;

    // search

    for(int d=loc_depth_start; d < loc_depth_lim; d++)
    {
        if(stok.stop_requested())
        {
            break;
        }

        ScoreVal_t lscore = negamax_ab(board, -EVAL_INF, EVAL_INF, d, stok);
        rm.score = lscore;
    }
}

int negamax_ab(Board board, int alpha, int beta, int depth_left, std::stop_token &stok)
{
    if(stok.stop_requested())
    {
        return 0;
    }

    // search lim check
    if( 0 == depth_left )
    {
        return quiesce(board, alpha, beta);
    }

    // generate moves for current node
    board.getMoves();

    for(Move_t move : board.moves)
    {
        ScoreVal_t local_score = -negamax_ab({board, move}, -beta, -alpha, depth_left-1, stok);
        if( local_score >= beta)
        {
            return beta; // hard beta-cutoff
        }
        if( local_score > alpha )
        {
            alpha = local_score;
        }
    }
    return alpha;
}

int quiesce(Board &board, int alpha, int beta)
{
    if(board.sideToMove())
    {
        return evaluate<true>(board);
    }
    else
    {
        return evaluate<false>(board);
    }
}
