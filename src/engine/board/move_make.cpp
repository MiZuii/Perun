#include "board_repr.h"

/* OPTIMS:
 - capture flag is called twice?
 - metatemplate the function for white move? (maybe metatemplate the whole board class)
*/

/* Method overview
1. save needed variables
2. process common flags clocks and other variables
3. if kingmove(castle flag) -> process all castle information 
    if the move accualy is a castle it is gonna be made and function
    will return early to not perform furher checks and calculations
    as most of them can be skipped due to nature of castle
4. Finish checking other flags and making appropriate stuff

*/
_Inline void Board::movePiece(Side playing_side, int source_square, int target_square, Piece source_piece)
{
    CLEAR_BIT(_occ_bitboards[playing_side], source_square);
    SET_BIT(_occ_bitboards[playing_side], target_square);

    CLEAR_BIT(_occ_bitboards[Side::BOTH], source_square);
    SET_BIT(_occ_bitboards[Side::BOTH], target_square);

    CLEAR_BIT(_piece_bitboards[source_piece], source_square);
    SET_BIT(_piece_bitboards[source_piece], target_square);
}

Board &Board::makeMove(Move_t move)
{

    _en_passant=NO_SQ;
    Piece source_piece = getSourcePiece(move);
    Side next_side = opositeSide(_side_to_move);
    int source_square = getSourceSquare(move);
    int target_square = getTargetSquare(move);

    if(_side_to_move == Side::BLACK)
    {
        _fullmove_clock += 1;
    }

    if(!GET_BIT(_piece_bitboards[source_piece], source_square))
    {
        /* Catches the case when queen was regarded as bishop/rook in a pin */
        source_piece = getColoredQueen(_side_to_move);
    }

    // move main piece
    movePiece(_side_to_move, source_square, target_square, source_piece);
    _side_to_move = next_side;

    /* ----------------------------- HANDLE CASTLING ---------------------------- */

    if(next_side == Side::BLACK)
    {
        if(_castle_rights & 0b1100)
        {
            if(getCastleFlag(move))
            {

                if(target_square == G1 && (_castle_rights & 0b1000))
                {
                    // kingside castle
                    movePiece(Side::WHITE, H1, F1, R);
                    CLEAR_BIT(_castle_rights, 3);
                    CLEAR_BIT(_castle_rights, 2);
                    return *this;
                }
                else if(target_square == C1 && (_castle_rights & 0b0100))
                {
                    // queenside castle
                    movePiece(Side::WHITE, A1, D1, R);
                    CLEAR_BIT(_castle_rights, 3);
                    CLEAR_BIT(_castle_rights, 2);
                    return *this;
                }

                CLEAR_BIT(_castle_rights, 3);
                CLEAR_BIT(_castle_rights, 2);
            }
            if(getRookFlag(move))
            {
                // clear castling if rook moved from it's original square
                if(source_square == H1)
                {
                    CLEAR_BIT(_castle_rights, 3);
                }
                else if(source_square == A1)
                {
                    CLEAR_BIT(_castle_rights, 2);
                }
            }
        }
    }
    else
    {
        if(_castle_rights & 0b0011)
        {
            if(getCastleFlag(move))
            {

                if(target_square == G8 && (_castle_rights & 0b0010))
                {
                    // kingside castle
                    movePiece(Side::BLACK, H8, F8, Piece::r);
                    CLEAR_BIT(_castle_rights, 1);
                    CLEAR_BIT(_castle_rights, 0);
                    return *this;
                }
                else if(target_square == C8 && (_castle_rights & 0b0001))
                {
                    // queenside castle
                    movePiece(Side::BLACK, A8, D8, Piece::r);
                    CLEAR_BIT(_castle_rights, 1);
                    CLEAR_BIT(_castle_rights, 0);
                    return *this;
                }

                CLEAR_BIT(_castle_rights, 1);
                CLEAR_BIT(_castle_rights, 0);
            }
            if(getRookFlag(move))
            {
                // clear castling if rook moved from it's original square
                if(source_square == H8)
                {
                    CLEAR_BIT(_castle_rights, 1);
                }
                else if(source_square == A8)
                {
                    CLEAR_BIT(_castle_rights, 0);
                }
            }
        }
    }

    /* -------------------------- OTHER FLAGS AND MOVES ------------------------- */

    if(getCaptureFlag(move) | getPawnPushFlag(move))
    {
        // reset no capture counter
        _halfmove_clock = 0;
    }

    if(getDoublePawnPushFlag(move))
    {
        if(next_side == Side::WHITE)
        {
            _en_passant = target_square + 8;
        }
        else
        {
            _en_passant = target_square - 8;
        }
    }

    // change bitboards according to move type
    if(getCaptureFlag(move))
    {
        for(Piece enemy_piece_type : (next_side == Side::WHITE ? whitePieces : blackPieces) )
        {
            // check which piece is not nececery as there cannot be two pieces on one square
            CLEAR_BIT(_piece_bitboards[enemy_piece_type], target_square);
            CLEAR_BIT(_occ_bitboards[next_side], target_square);
        }
    }

    if(getEnPassantFlag(move))
    {
        if(next_side == Side::BLACK)
        {
            CLEAR_BIT(_piece_bitboards[Piece::p], target_square-8);
            CLEAR_BIT(_occ_bitboards[next_side], target_square-8);
        }
        else
        {
            CLEAR_BIT(_piece_bitboards[Piece::P], target_square+8);
            CLEAR_BIT(_occ_bitboards[next_side], target_square+8);
        }
    }

    int promotion_piece = getPromotionPiece(move);
    if(promotion_piece != Piece::no_piece)
    {
        if(next_side == Side::BLACK)
        {
            CLEAR_BIT(_piece_bitboards[Piece::P], target_square);
            SET_BIT(_piece_bitboards[promotion_piece], target_square);
        }
        else
        {
            CLEAR_BIT(_piece_bitboards[Piece::p], target_square);
            SET_BIT(_piece_bitboards[promotion_piece], target_square);
        }
    }

    return *this;
}

Move_t Board::createAmbiguousMove(Move_t move)
{
    Piece source_piece = no_piece;
    for(Piece try_piece : allPieces)
    {
        if(GET_BIT(_piece_bitboards[try_piece], getSourceSquare(move)))
        {
            source_piece = try_piece;
            break;
        }
    }

    // checks if source piece is correctly specified
    assert( source_piece != no_piece );
    if(_side_to_move == WHITE)
    {
        assert(isWhitePiece(source_piece));
    }
    else
    {
        assert(isBlackPiece(source_piece));
    }

    Piece promotion_piece = getPromotionPiece(move);
    if( promotion_piece != no_piece )
    {
        promotion_piece = convertPiece(getPieceType(promotion_piece), _side_to_move);
        
        // check if promotion piece is correct
        assert(getPieceType(promotion_piece) == QUEEN ||
            getPieceType(promotion_piece) == BISHOP ||
            getPieceType(promotion_piece) == KNIGHT ||
            getPieceType(promotion_piece) == ROOK);
    }

    bool capture_flag = false;
    if(_side_to_move == WHITE)
    {
        // check if move doesn't capture it's own piece
        assert(GET_BIT(_occ_bitboards[WHITE], getTargetSquare(move)) == 0);

        capture_flag = GET_BIT(_occ_bitboards[BLACK], getTargetSquare(move));
    }
    else if(_side_to_move == BLACK)
    {
        // check if move doesn't capture it's own piece
        assert(GET_BIT(_occ_bitboards[BLACK], getTargetSquare(move)) == 0);

        capture_flag = GET_BIT(_occ_bitboards[WHITE], getTargetSquare(move));
    }

    bool double_pawn_push_flag = false;
    bool normal_pawn_push_flag = false;
    if( getPieceType(source_piece) == PAWN )
    {
        int diff = abs(getTargetSquare(move) / 8 - getSourceSquare(move) / 8);
        double_pawn_push_flag = (diff == 2);
        normal_pawn_push_flag = (diff == 1);
    }

    bool en_passant_flag = false;
    if( getPieceType(source_piece) == PAWN && getTargetSquare(move) == _en_passant && _en_passant != NO_SQ )
    {
        en_passant_flag = true;
    }

    bool castle_flag = false;
    if( getPieceType(source_piece) == KING )
    {
        int diff = abs((getTargetSquare(move) % 8) - (getSourceSquare(move) % 8));
        castle_flag = (diff > 1);
    }

    bool rook_move_flag = (getPieceType(source_piece) == ROOK);

    return createMove(getSourceSquare(move), getTargetSquare(move),
        source_piece, promotion_piece, capture_flag, double_pawn_push_flag, 
        en_passant_flag, castle_flag, rook_move_flag, normal_pawn_push_flag);
}
