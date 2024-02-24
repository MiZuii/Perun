#include "search.h"

#include "../interface/interface.h"

/* Main search function. Creates the search threads and needed structures and
loops infinitly managind search end */
void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    // get moves for all root moves
    board.getMoves();

    // initiate needed structures
    std::vector<std::jthread>   *root_threads               = new std::vector<std::jthread>;
    std::atomic<unsigned int>   completion_counter          = 0;
    const unsigned int          number_of_search_threads    = board.moves.size();
    const Side                  player                      = board.sideToMove();
    const auto                  loc_start                   = std::chrono::steady_clock::now();

    /* Deduce time control for the current search */
    if( TIME_LIM == args.search_type && -1 == args.time_lim)
    {
        args.time_lim = movetime_deduction(args, player);
    }

    /* start time ocunt (after this line till creation of search threads there should be nothing) */
    {
        std::lock_guard<std::mutex> end_guard(engmtx);
        engr.tstart = std::chrono::steady_clock::now();
    }

    /* Initiate the search threads for every root move */
    for(Move_t move : board.moves)
    {
        root_threads->push_back(std::jthread(_search,
            Board(board, move), move, std::ref(engr), 
            std::ref(engmtx), args, player, std::ref(completion_counter),
            std::ref(move_hist)));
    }

    
    while(true)
    {
        /* If stop was requested (externally or by internal time controll) delete
        the root threads vector which causes all search threads to stop and increment
        completion_counter. */
        if(stok.stop_requested())
        {
            delete root_threads;
            break;
        }

        /* If all threads finished searching end the main loop */
        if(number_of_search_threads <= completion_counter.load())
        {
            break;
        }

        /* Check if time for search ended if the limit type is set to TIME_LIM */
        if( args.search_type == TIME_LIM && std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::steady_clock::now() - loc_start).count() >= args.time_lim)
        {
            // issue stop request through main search thread stop token
            Engine::stop();
        }

        /* Sleep 0 for puting the thread at the end of scheduling queue */
        std::this_thread::sleep_for(0ms);
    }

    /* Update necessery flags for protocol thread */
    {
        std::lock_guard<std::mutex> end_guard(engmtx);
        engr.finished = true;
    }

    Engine::engine_running = false;
}


/* Root Move thread search worker. */
void _search(std::stop_token stok, Board board, Move_t move,
    EngineResults &engr, std::mutex &engmtx, SearchArgs args, 
    const Side player, std::atomic<unsigned int> &comp_counter, 
    const std::vector<Move_t> &move_hist)
{

    // initiate search type and parameters based on the RootMove and SearchArgs information
    const int loc_depth_lim     = args.search_type == DEPTH_LIM ? args.depth_lim : SEARCH_INF;
    const int loc_depth_start   = args.depth_start;
    RootMove  rm                = {move, 0, 0, RootMove::KMS_NUM - 1, {},
                                   {std::vector<Move_t>(move_hist), move_hist.size() + 1},
                                   0, player, stok};

    /* push back thread specific root move as latest history move */
    rm.mstack.moves.push_back(move);

    
    /* Iterative deepening search loop */
    for(int d=loc_depth_start; d < loc_depth_lim; d++)
    {
        rm.eval_depth = d;
        ScoreVal_t lscore = negamax_ab(board, -EVAL_INF, EVAL_INF, d, rm);

        /* If the search returned because of stop token the lscore is invalid 
        and the search should be stopped imidiately */
        if(stok.stop_requested())
        {
            break;
        }

        /* update RootMove structure localy and non finish engine result values
        through the update_engres function */
        rm.score = board.sideToMove() == WHITE ? lscore : -lscore;
        update_engres(rm, engr, engmtx);

        /* debug print pv */
        // std::string pvstr;
        // std::vector<Move_t> pv;
        // TT::pv_probe(board, d, pv);
        // for(auto move : pv)
        // {
        //     pvstr += " " + Board::moveToStringShort(move);
        // }
        // std::cerr << "move: " + Board::moveToStringShort(rm.root_move) + " score: " + std::to_string(rm.score) + " pv: " + pvstr + "\n";
    }

    /* Finally update thre search results and increment completion counter
    for proper search termination */
    update_engres(rm, engr, engmtx, true);
    comp_counter++;
}



int negamax_ab(Board board, int alpha, int beta, int depth_left, RootMove &rm)
{
    /* Return if search is terminated */
    if(rm.stok.stop_requested())
    {
        return 0;
    }

    /* Probe hashtable */
    ScoreVal_t local_score = TT::probe(board, alpha, beta, rm.eval_depth);
    if( UNKNOWN_VAL != local_score )
    {
        return local_score;
    }

    /* If reached initial search lim run quiesce */
    if( 0 == depth_left )
    {
        return quiesce(board, alpha, beta, 15, rm);
    }

    // generate moves for current node
    board.getMoves();

    /* Return with proper value if game ended with checkmate or stalemate */
    if(board.moves.empty())
    {
        return board.getCheckers() ? -EVAL_INF : 0;
    }

    // run move ordering
    std::sort(board.moves.begin(), board.moves.end(), MoveOrder(board, rm, depth_left));

    /* Create node type variable for remembering which node will be writen to tt */
    NodeType nt = ALPHA;
    Move_t bm   = 0;

    /* Run negamax_ab logic */
    for(Move_t move : board.moves)
    {
        rm.mstack.moves.push_back(move);
        local_score = -negamax_ab({board, move}, -beta, -alpha, depth_left-1, rm);
        rm.mstack.moves.pop_back();

        if( local_score >= beta)
        {
            /* Update killermoves array if in bounds of depth */
            if( !getCaptureFlag(move) && depth_left <= RootMove::MAX_PLY)
            {
                rm.latest_km_index = (rm.latest_km_index + 1) % RootMove::KMS_NUM; // cyclic index update
                rm.killer_moves[rm.latest_km_index][depth_left-1] = move;
            }

            TT::write(board, beta, BETA, move, rm.eval_depth);
            return beta;
        }
        if( alpha < local_score )
        {
            nt    = EXACT;
            bm    = move;
            alpha = local_score;
        }
    }
    TT::write(board, alpha, nt, bm, rm.eval_depth);
    return alpha;
}



int quiesce(Board board, int alpha, int beta, Depth_t depth_left, RootMove &rm)
{
    rm.nc++; // node count increment

    /* Return if search terminated */
    if(rm.stok.stop_requested())
    {
        return 0;
    }

    /* generate moves and sort them for current node.
    The RootMove::MAX_PLY + 1 is set to remove killermoves
    in the ordering hashmap */
    board.getMoves();
    std::sort(board.moves.begin(), board.moves.end(), MoveOrder(board, rm, RootMove::MAX_PLY + 1));

    // current node static evaluation
    ScoreVal_t stand_pat = (board.sideToMove() == WHITE) ? evaluate<true>(board) : -evaluate<false>(board);

    /* Check if max depth was reached by the quiesce */
    if( depth_left == 0 )
    {
        return stand_pat;
    }


    /* Run quiesce logic*/
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
        /* Search only captures or check evasions */
        if(!getCaptureFlag(move) && !board.getCheckers())
        {
            continue;
        }

        rm.mstack.moves.push_back(move);
        ScoreVal_t local_score = -quiesce({board, move}, -beta, -alpha, depth_left-1, rm);
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

void update_engres(RootMove &rm, EngineResults &engr, std::mutex &engmtx, bool final)
{
    std::lock_guard<std::mutex> end_guard(engmtx);

    bool new_data_flag = false;

    // score update
    if(final && (((rm.player == WHITE) ? (rm.score > engr.best_score) : (rm.score < engr.best_score)) || engr.best_move == 0) 
             && rm.eval_depth != 1)
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

    // wakeup if new data 
    if(new_data_flag)
    {
        engr.new_data.notify_all();
    }
}
