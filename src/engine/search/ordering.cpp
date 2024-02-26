#include "ordering.h"
#include "search.h"

MoveOrder::MoveOrder(Board &board, SearchThread &st, int d) : _board(board), _st(st), _d(d)
{
    /* Reserve space for all moves compare values */
    _mvalmap.reserve(_board.moves.size());

    /* Get entry from transposition table previous generation */
    TTItem *probe = TT::raw_probe(board, st.generation - 1);
    Move_t best_move = 0;
    if( nullptr != probe && probe->type == EXACT )
    {
        best_move = probe->bm;
    }

    /* Iterate through every move and assign an ordering value
    to the map under the moves index */
    for(Move_t move : _board.moves)
    {
        /* Get target piece type and retrive value from mvv_lva table */
        Piece p = no_piece;
        for(Piece posp : getColoredPieces(_board.sideToMove()))
        {
            if(GET_BIT(_board._piece_bitboards[posp], getTargetSquare(move)))
            {
                p = posp;
                break;
            }
        }

        /* Accomulator variable for the final move score filled with initial lookup */
        int m_val = mvv_lva[getSourcePiece(move)][p];

        /* Calculate val based on the killermoves */
        if( !getCaptureFlag(move) && _d <= SearchThread::MAX_PLY)
        {
            for(int i=SearchThread::KMS_NUM; i > 0; i--)
            {
                if(_st.killer_moves[(_st.latest_km_index + i)%SearchThread::KMS_NUM][_d-1] == move)
                {
                    // killer move was found -> assign value based on the move age
                    m_val += (50 / SearchThread::KMS_NUM)*(i - 1) + 50;
                    break;
                }
            }
        }

        /* Check if move is maching transposition table best move */
        if( move == best_move )
        {
            m_val = INT_MAX;
        }

        /* fill the hashmap with final value*/
        _mvalmap[move] = m_val;
    }
}

bool MoveOrder::operator()(const Move_t &a, const Move_t &b)
{
    return _mvalmap[a] < _mvalmap[b];
}