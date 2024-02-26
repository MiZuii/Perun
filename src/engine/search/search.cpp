#include "search.h"

#include "../interface/interface.h"

/* Main search function. Creates the search threads and needed structures and
loops infinitly managind search end */
void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    // initiate needed structures
    const Side player    = board.sideToMove();
    const auto loc_start = std::chrono::steady_clock::now();
    constexpr int workers_num = 8; // should be deduced from cpu information

    /* Write search start time to the results */
    {
        std::lock_guard<std::mutex> eng_guard(engmtx);
        engr.tstart = loc_start;
    }

    /* Deduce time control for the current search */
    if( TIME_LIM == args.search_type && -1 == args.time_lim)
    {
        args.time_lim = movetime_deduction(args, player);
    }

    /* Run main search thread */
    const auto mthread = new SearchThread<true>();
    mthread->run();
    
    while(true)
    {
        /* If stop was requested (externally or by internal time controll) delete
        the root threads vector which causes all search threads to stop and increment
        completion_counter. */
        if(stok.stop_requested())
        {
            delete mthread;
            break;
        }

        /* If all threads finished searching end the main loop */
        if( mthread->isFinished() )
        {
            delete mthread;
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

/* -------------------------------------------------------------------------- */
/*                                 SPLIT POINT                                */
/* -------------------------------------------------------------------------- */

SplitPoint::SplitPoint(SearchThread &owner, Board &board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left) :
    _owner(owner), board(board), alpha(alpha), beta(beta), depth_left(depth_left), _last_todo_idx(0)
{
    Move_t pv_move = _owner.pv->at(board.getPositionDepth()-_owner.pv_init_depth);
    _moves.reserve(board.moves.size()-1);

    for(auto move : board.moves)
    {
        if( move == pv_move )
        {
            _main_move_state.move = pv_move;
            _main_move_state.state = WORKING;
            continue;
        }

        _moves.emplace_back(move, 0, TODO);
    }
}

MoveState *SplitPoint::get_work()
{
    if(_last_todo_idx == _moves.size())
    {
        return nullptr;
    }

    MoveState *ms = &_moves[_last_todo_idx++];
    ms->state = WORKING;
    return ms;
}

bool SplitPoint::finished()
{
    for(MoveState &ms : _moves)
    {
        if(ms.state != DONE)
        {
            return false;
        }
    }

    return true;
}

void SplitPoint::work_done(MoveState *ms, ScoreVal_t res)
{
    ms->score = res;
    ms->state = WAITING;
    _owner.pidle.notify_one(); // this variable is thread local soo one is all
}

/* -------------------------------------------------------------------------- */
/*                                SEARCH THREAD                               */
/* -------------------------------------------------------------------------- */

void SearchThread::iterate(Board board, SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    // initiate search type and parameters based on the SearchArgs information
    const int loc_depth_lim     = args.search_type == DEPTH_LIM ? args.depth_lim : SEARCH_INF;
    const int loc_depth_start   = args.depth_start;
    SearchThread::pv_init_depth = board.getPositionDepth();
    std::vector<Move_t>           pv;

    
    /* Iterative deepening search loop */
    for(int d=loc_depth_start; d < loc_depth_lim; d++)
    {
        /* Init this depth search */
        SearchThread::generation = d;
        SearchThread::pv = &pv;
        SearchThread::split_points.clear();
        
        ScoreVal_t lscore = negamax(board, -EVAL_INF, EVAL_INF, d);

        /* Check the search stop condition */
        if(over)
        {
            break;
        }

        /* Retrive the pv from tt */
        pv.clear();
        TT::pv_probe(board, d, pv);
    }

    /* Finally update thre search results and increment completion counter
    for proper search termination */
    update_engres(rm, engr, engmtx, true);
}

void SearchThread::create_worker()
{
    SearchThread this_thread;
    this_thread.work();
}

void SearchThread::create_main()
{
    SearchThread this_thread;
    this_thread.iterate();
}

void SearchThread::work()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock{mutex};
        bool work_done_flag = false;
        ScoreVal_t result = 0;

        if(!split_points.empty())
        {
            SplitPoint &sp = split_points.back();
            MoveState  *ms = nullptr;
            ScoreVal_t  res;

            if((ms = sp.get_work()) != nullptr)
            {
                lock.release();
                res = -negamax({sp.board, ms->move}, 
                                -sp.beta, -sp.alpha, 
                                sp.depth_left-1);
                lock.lock();
                sp.work_done(ms, res);
                work_done_flag = true;
            }
        }

        if(!work_done_flag)
        {
            idle.wait(lock);
        }
    }
}

ScoreVal_t SearchThread::negamax(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left)
{
    nc++; // node count increment

    /* Return if search is terminated */
    if(over)
    {
        return 0;
    }

    /* Probe hashtable */
    ScoreVal_t local_score = TT::probe(board, alpha, beta, generation);
    if( UNKNOWN_VAL != local_score )
    {
        return local_score;
    }

    /* If reached initial search lim run quiesce */
    if( 0 == depth_left )
    {
        return quiesce(board, alpha, beta, 15);
    }

    // generate moves for current node
    board.getMoves();

    // run move ordering
    std::sort(board.moves.begin(), board.moves.end(), MoveOrder(board, *this, depth_left));

    /* -------------------------------- PV SPLIT -------------------------------- */

    if(TT::is_pv(board, generation))
    {
        /* Initiate split point */
        std::unique_lock lock(mutex);
        split_points.emplace_back(*this, board, alpha, beta, depth_left);
        SplitPoint &sp = split_points.back();

        /* Go deeper on the main move */
        lock.unlock();
        ScoreVal_t main_val = -negamax({board, sp._main_move_state.move},
                                        -beta, -alpha, depth_left-1);
        lock.lock();

        /* Update SplitPoint opening it for workers */
        sp.alpha = main_val;
        sp._main_move_state.state = DONE;
        idle.notify_all();

        /* Create node type variable for remembering which node will be writen to tt */
        NodeType nt = ALPHA;
        Move_t bm   = 0;

        /* Loop updating values from workers */
        while(true)
        {
            if(!sp.finished())
            {
                break;
            }

            pidle.wait(lock);

            for(MoveState &ms : sp._moves)
            {
                /* Skip for states that cannot be reviewed yet */
                if( ms.state != WAITING )
                {
                    continue;
                }

                if( ms.score >= sp.beta)
                {
                    /* Update killermoves array if in bounds of depth */
                    if( !getCaptureFlag(ms.move) && sp.depth_left <= SearchThread::MAX_PLY)
                    {
                        latest_km_index = (latest_km_index + 1) % SearchThread::KMS_NUM; // cyclic index update
                        killer_moves[latest_km_index][depth_left-1] = ms.move;
                    }

                    TT::write(board, sp.beta, BETA, ms.move, generation);
                    return sp.beta;
                }
                if( sp.alpha < ms.score )
                {
                    nt       = EXACT;
                    bm       = ms.move;
                    sp.alpha = ms.score;
                }
            }
        }

        if( bm != 0)
        {
            // the principal variation changed
            TT::write(board, sp.alpha, nt, bm, generation);
        }
        return sp.alpha;
    }

    /* ------------------------------ PV SPLIT OVER ----------------------------- */

    /* Return with proper value if game ended with checkmate or stalemate */
    if(board.moves.empty())
    {
        return board.getCheckers() ? -EVAL_INF : 0;
    }

    /* Create node type variable for remembering which node will be writen to tt */
    NodeType nt = ALPHA;
    Move_t bm   = 0;

    /* Run negamax_ab logic */
    for(Move_t move : board.moves)
    {
        local_score = -negamax({board, move}, -beta, -alpha, depth_left-1);

        if( local_score >= beta)
        {
            /* Update killermoves array if in bounds of depth */
            if( !getCaptureFlag(move) && depth_left <= SearchThread::MAX_PLY)
            {
                latest_km_index = (latest_km_index + 1) % SearchThread::KMS_NUM; // cyclic index update
                killer_moves[latest_km_index][depth_left-1] = move;
            }

            TT::write(board, beta, BETA, move, generation);
            return beta;
        }
        if( alpha < local_score )
        {
            nt    = EXACT;
            bm    = move;
            alpha = local_score;
        }
    }
    TT::write(board, alpha, nt, bm, generation);
    return alpha;
}

ScoreVal_t SearchThread::quiesce(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left)
{
    nc++; // node count increment

    /* Return if search terminated */
    if(over)
    {
        return 0;
    }

    /* generate moves and sort them for current node.
    The RootMove::MAX_PLY + 1 is set to remove killermoves
    in the ordering hashmap */
    board.getMoves();
    std::sort(board.moves.begin(), board.moves.end(), MoveOrder(board, *this, SearchThread::MAX_PLY + 1));

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

        ScoreVal_t local_score = -quiesce({board, move}, -beta, -alpha, depth_left-1);

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

/* -------------------------------------------------------------------------- */
/*                                    MISC                                    */
/* -------------------------------------------------------------------------- */

int movetime_deduction(const SearchArgs args, const Side player)
{
    // returns time in milliseconds
    return 5000;
}

void update_engres(SearchThread &rm, EngineResults &engr, std::mutex &engmtx, bool final)
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
