#include "tt.h"

void TT::init()
{
    _tt = static_cast<TTItem*>(calloc(_size, ITEM_SIZE));

    if(nullptr == _tt)
    {
        // error exit
        exit(1);
    }
}

void TT::resize(size_t new_size)
{
    _tt = static_cast<TTItem*>(realloc(_tt, new_size));
    if(nullptr == _tt)
    {
        //error exit
        exit(1);
    }
}

void TT::destroy()
{
    if(nullptr != _tt)
    {
        free(_tt);
    }
}

ScoreVal_t TT::probe(Board &board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t gen)
{
    TTItem *probe = &TT::_tt[board.getHash() % TT::_size];

    if( probe->key != board.getHash() || probe->gen != (gen % 32) )
    {
        return UNKNOWN_VAL;
    }

    if( probe->depth < board.getPositionDepth() )
    {
        // remeber best move
        return UNKNOWN_VAL;
    }

    if (probe->type == EXACT) return probe->score;
    if ((probe->type == ALPHA) && (probe->score <= alpha)) return alpha;
    if ((probe->type == BETA) && (probe->score >= beta)) return beta;

    return UNKNOWN_VAL;
}

TTItem *TT::raw_probe(Board &board, Depth_t gen)
{
    TTItem *probe = &TT::_tt[board.getHash() % TT::_size];

    if( probe->key != board.getHash() ||
        probe->gen != (gen % 32) ||
        probe->depth < board.getPositionDepth() )
    {
        return nullptr;
    }
    
    return probe;
}

void TT::write(Board &board, ScoreVal_t score, NodeType nt, Move_t bm, Depth_t gen)
{
    TTItem *probe = &TT::_tt[board.getHash() % TT::_size];

    probe->key = board.getHash();
    probe->score = score;
    probe->depth = board.getPositionDepth();
    probe->type = nt;
    probe->bm = bm;
    probe->gen = gen % 32;
}

void TT::pv_probe(Board board, Depth_t gen, std::vector<Move_t> &pvv)
{
    Board b{board};

    while(true)
    {
        // get new position moves
        b.getMoves();

        // pv move variable
        Move_t pv_move = 0;
        ScoreVal_t pv_score = -EVAL_INF;

        // probe moves searching for pv move
        for(auto move : b.moves)
        {
            Board mb{b, move};
            TTItem *probe = raw_probe(mb, gen);

            if(nullptr == probe)
            {
                continue;
            }

            if(probe->type == EXACT && probe->score > pv_score)
            {
                pv_move = move;
                pv_score = probe->score;
            }
        }

        if(pv_move == 0)
        {
            break;
        }

        // push new pv move and go one iter forward
        pvv.push_back(pv_move);
        b.makeMove(pv_move);
    }
}

/* ----------------------------- HASH KEYS FILL ----------------------------- */

void fill_hash_arrays()
{
    for(int i=0; i<12; i++) {
        for(int j=0; j<64; j++) {
            piece_hash_keys[i][j] = XORSHIFT::get();
        }
    }

    for(int i=0; i<64; i++) {
        en_passant_hash_keys[i] = XORSHIFT::get();
    }
    en_passant_hash_keys[NO_SQ] = 0;

    for(int i=0; i<16; i++) {
        castling_hash_keys[i] = XORSHIFT::get();
    }

    side_hash_key = XORSHIFT::get(); 
}
