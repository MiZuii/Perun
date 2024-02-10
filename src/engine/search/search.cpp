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

    std::reference_wrapper<RootMove> best_move(root_moves[0]);

    // controll loop
    while(!stok.stop_requested())
    {
        for(RootMove &rm : root_moves)
        {
            if(rm.score > bs)
            {
                bs = rm.score;
                best_move = std::reference_wrapper<RootMove>(rm);
            }
        }

        if(previous_bs != bs)
        {
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

}

int negamax_ab(Board board, int alpha, int beta, int depth_left)
{
    // search lim check
    if( 0 == depth_left )
    {
        return quiesce(board, alpha, beta);
    }

    // generate moves for current node
    board.getMoves();

    for(Move_t move : board.moves)
    {
        ScoreVal_t local_score = -negamax_ab({board, move}, -beta, -alpha, depth_left-1);
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
    return evaluate(board);
}
