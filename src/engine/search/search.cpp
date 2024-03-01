#include "search.h"

#include "../interface/interface.h"

/* -------------------------------------------------------------------------- */
/*                                   SEARCH                                   */
/* -------------------------------------------------------------------------- */

/* Main search function. Creates the search threads and needed structures and
loops infinitly managind search end */
void search(std::stop_token stok, Board board, const std::vector<Move_t> move_hist, 
    SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    // initiate needed structures
    const Side player    = board.sideToMove();
    const auto loc_start = std::chrono::steady_clock::now();
    constexpr int workers_num = 8; // should be deduced from cpu information

    /* Clear SearchThread static variables */
    SearchThread::clear_statics();

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

    /* Create workers vector */
    std::vector<dthread> *workers = new std::vector<dthread>();
    for(int worker_id=0; worker_id < workers_num; worker_id++)
    {
        workers->push_back(dthread(SearchThread::create_worker));
    }

    /* Run main search thread */
    const dthread *mthread = new dthread(SearchThread::create_main,
                                         board, args, std::ref(engr), std::ref(engmtx));
    
    while(true)
    {
        if(stok.stop_requested())
        {
            SearchThread::signal_end();
            delete workers;
            delete mthread;
            break;
        }

        /* If all threads finished searching end the main loop */
        if( SearchThread::finished() )
        {
            delete workers;
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
    std::lock_guard end_guard(engmtx);
    engr.finished = true;
    engr.new_data.notify_all();
    Engine::engine_running = false;
}

/* -------------------------------------------------------------------------- */
/*                                 SPLIT POINT                                */
/* -------------------------------------------------------------------------- */

SplitPoint::SplitPoint(SearchThread &owner, Board &board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left) :
    _owner(owner), _last_todo_idx(0), board(board), alpha(alpha), beta(beta), depth_left(depth_left)
{
    Move_t pv_move = (_owner.pv->empty() || board.getPositionDepth()-_owner.pv_init_depth >= _owner.pv->size()) ?
        board.moves[0] : _owner.pv->at(board.getPositionDepth()-_owner.pv_init_depth);
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
    std::unique_lock              lock(mutex);
    pv_init_depth               = board.getPositionDepth();
    std::vector<Move_t>           lpv;
    pv                          = &lpv;

    
    /* Iterative deepening search loop */
    for(int d=loc_depth_start; d < loc_depth_lim; d++)
    {
        std::cerr << "new iter-----------\n";
        /* Init this depth search */
        SearchThread::generation = d;
        SearchThread::split_points.clear();

        /* Propagate pv onto the TT */
        TT::pv_propagate(board, d, lpv);
        
        lock.unlock();
        ScoreVal_t lscore = negamax(board, -EVAL_INF, EVAL_INF, d);
        lscore = board.sideToMove() == WHITE ? lscore : -lscore;
        lock.lock();

        /* Check the search stop condition */
        if(over)
        {
            break;
        }

        /* Retrive the pv from tt */
        lpv.clear();
        TT::pv_probe(board, d, lpv);
        update_engres(lpv, lscore, d, engr, engmtx);
    }

    lock.unlock(); // signal end uses the mutex!
    SearchThread::signal_end();
}

void SearchThread::create_worker()
{
    SearchThread this_thread;
    this_thread.work();
}

void SearchThread::create_main(Board board, SearchArgs args, EngineResults &engr, std::mutex &engmtx)
{
    SearchThread this_thread;
    this_thread.iterate(board, args, engr, engmtx);
}

void SearchThread::signal_end()
{
    std::lock_guard lg(mutex);
    idle.notify_all();
    over = true;
}

unsigned int SearchThread::probe_nc()
{
    std::lock_guard lg(mutex);
    return nc;
}

void SearchThread::clear_statics()
{
    nc            = 0;
    pv            = nullptr;
    pv_init_depth = 0;
    generation    = 0;
    over          = false;

    split_points.clear();
}

void SearchThread::work()
{
    std::unique_lock lock(mutex);

    while(true)
    {
        bool work_done_flag = false;

        if(!split_points.empty())
        {
            SplitPoint &sp = split_points.back();
            MoveState  *ms = nullptr;
            ScoreVal_t  res;

            if((ms = sp.get_work()) != nullptr)
            {
                lock.unlock();
                res = -negamax({sp.board, ms->move}, 
                                -sp.beta, -sp.alpha,
                                sp.depth_left-1);
                lock.lock();
                sp.work_done(ms, res);

                // node count update
                nc += lnc;
                lnc = 0;

                work_done_flag = true;
            }
        }

        if(!work_done_flag && !over)
        {
            idle.wait(lock);
        }

        if(over)
        {
            break;
        }
    }
}

ScoreVal_t SearchThread::negamax(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left)
{
    lnc++; // node count increment

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
        std::cerr << "new splitpoint>>>>>>>>>>>>>>>>>>>>>>>>\n";
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
        NodeType nt        = ALPHA;
        Move_t bm          = 0;
        ScoreVal_t ret_val = sp.alpha;

        /* Loop updating values from workers */
        while(true)
        {
            if(sp.finished() || over)
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

                    TT::write(board, sp.beta, false, BETA, ms.move, generation);
                    ret_val = sp.beta;
                    bm = 0;
                    break;
                }
                if( sp.alpha < ms.score )
                {
                    nt       = EXACT;
                    bm       = ms.move;
                    sp.alpha = ms.score;
                    ret_val  = ms.score;
                }

                /* Check that the ms was reviewed */
                ms.state = DONE;
            }
        }

        if( bm != 0)
        {
            // the principal variation changed
            TT::write(board, sp.alpha, true, nt, bm, generation);
        }

        /* update nc */
        nc += lnc;
        lnc = 0;
        split_points.pop_back();

        return ret_val;
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

            TT::write(board, beta, false, BETA, move, generation);
            return beta;
        }
        if( alpha < local_score )
        {
            nt    = EXACT;
            bm    = move;
            alpha = local_score;
        }
    }
    TT::write(board, alpha, false, nt, bm, generation);
    return alpha;
}

ScoreVal_t SearchThread::quiesce(Board board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t depth_left)
{
    lnc++; // node count increment

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

void update_engres(std::vector<Move_t> &pv, ScoreVal_t score, Depth_t gen, EngineResults &engr, std::mutex &engmtx)
{
    std::lock_guard<std::mutex> end_guard(engmtx);

    engr.pv = pv;
    engr.pv_score = score;
    engr.generation = gen;
    engr.new_pv = true;

    engr.new_data.notify_all();
}
