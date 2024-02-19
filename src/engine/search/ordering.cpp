#include "ordering.h"

MoveOrder::MoveOrder(Board &board) : _board(board)
{
    // fill hashmap
    // resize because size is known
    _pmap.reserve(_board.moves.size());

    // proper fill
    for(Move_t move : _board.moves)
    {
        Piece p = no_piece;

        for(Piece posp : getColoredPieces(_board.sideToMove()))
        {
            if(GET_BIT(_board._piece_bitboards[posp], getTargetSquare(move)))
            {
                p = posp;
                break;
            }
        }

        _pmap[move] = p;
    }
}

bool MoveOrder::operator()(const Move_t &a, const Move_t &b)
{
    Piece asp = getSourcePiece(a),
          bsp = getSourcePiece(b),
          atp = _pmap[a],
          btp = _pmap[b];

    return mvv_lva[asp][atp] < mvv_lva[bsp][btp];
}