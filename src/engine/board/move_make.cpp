#include "board_repr.h"
#include "../tt/tt.h"

_ForceInline void Board::movePiece(Side playing_side, int source_square, int target_square, Piece source_piece)
{
    _hash ^= piece_hash_keys[source_piece][source_square];      // unhash piece from prev square
    _hash ^= piece_hash_keys[source_piece][target_square];      // hash piece on new square

    CLEAR_BIT(_occ_bitboards[playing_side],   source_square);   // move on personal occ bb
    SET_BIT  (_occ_bitboards[playing_side],   target_square);   //

    CLEAR_BIT(_occ_bitboards[Side::BOTH],     source_square);   // move on merged occ bb
    SET_BIT  (_occ_bitboards[Side::BOTH],     target_square);   //

    CLEAR_BIT(_piece_bitboards[source_piece], source_square);   // move on piece bb
    SET_BIT  (_piece_bitboards[source_piece], target_square);   //
}

_ForceInline void Board::capturePiece(Piece piece_to_capture, int capture_square, Side captured_side)
{
    _hash ^= piece_hash_keys[piece_to_capture][capture_square];     // unhash captured piece (hashing of new piece happens in movePiece)
    CLEAR_BIT(_piece_bitboards[piece_to_capture], capture_square);  // clear captured piece from personal bb
    CLEAR_BIT(_occ_bitboards[captured_side],      capture_square);  // clear captured piece from sided occ bb
}

_ForceInline void Board::captureEnPassant(Piece sided_pawn, int capture_square, Side captured_side)
{
    _hash ^= piece_hash_keys[sided_pawn][capture_square];       // unhash captured pawn
    CLEAR_BIT(_piece_bitboards[sided_pawn],  capture_square);   // clear pawn personal bb
    CLEAR_BIT(_occ_bitboards[captured_side], capture_square);   // clear side bb
    CLEAR_BIT(_occ_bitboards[Side::BOTH],    capture_square);   // clear both occ bb
}

_ForceInline void Board::castle(int to_clear)
{
    _hash ^= castling_hash_keys[_castle_rights];    // unhash previous castle rights
    CLEAR_BITS(_castle_rights, to_clear);           // set new castle rights
    _hash ^= castling_hash_keys[_castle_rights];    // hash new castle rights
}

_ForceInline void Board::promote(Piece sided_pawn, Piece promotion_piece, int promotion_square)
{
    _hash ^= piece_hash_keys[sided_pawn][promotion_square];         // unhash promoting pawn
    CLEAR_BIT(_piece_bitboards[sided_pawn], promotion_square);      // clear pawn from personal bb
    _hash ^= piece_hash_keys[promotion_piece][promotion_square];    // hash new promoted piece
    SET_BIT(_piece_bitboards[promotion_piece], promotion_square);   // set new piece on personal bb
}



Board &Board::makeMove(Move_t move)
{
    
    /* unhash previous en_passant. hash_key of NO_SQ is 0 soo no need to check that */
    _hash ^= en_passant_hash_keys[_en_passant];

    _en_passant         = NO_SQ;
    Piece source_piece  = getSourcePiece(move);
    Side next_side      = opositeSide(_side_to_move);
    int source_square   = getSourceSquare(move);
    int target_square   = getTargetSquare(move);

    if(_side_to_move == Side::BLACK)
    {
        _fullmove_clock += 1;
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
                    castle(0b1100);
                    // CLEAR_BIT(_castle_rights, 3); old
                    // CLEAR_BIT(_castle_rights, 2);
                    return *this;
                }
                else if(target_square == C1 && (_castle_rights & 0b0100))
                {
                    // queenside castle
                    movePiece(Side::WHITE, A1, D1, R);
                    castle(0b1100);
                    // CLEAR_BIT(_castle_rights, 3); old
                    // CLEAR_BIT(_castle_rights, 2);
                    return *this;
                }

                castle(0b1100);
                // CLEAR_BIT(_castle_rights, 3); old
                // CLEAR_BIT(_castle_rights, 2);
            }
            if(getRookFlag(move))
            {
                // clear castling if rook moved from it's original square
                if(source_square == H1)
                {
                    castle(0b1000);
                    // CLEAR_BIT(_castle_rights, 3); old
                }
                else if(source_square == A1)
                {
                    castle(0b0100);
                    // CLEAR_BIT(_castle_rights, 2); old
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
                    castle(0b0011);
                    // CLEAR_BIT(_castle_rights, 1); old
                    // CLEAR_BIT(_castle_rights, 0);
                    return *this;
                }
                else if(target_square == C8 && (_castle_rights & 0b0001))
                {
                    // queenside castle
                    movePiece(Side::BLACK, A8, D8, Piece::r);
                    castle(0b0011);
                    // CLEAR_BIT(_castle_rights, 1); old
                    // CLEAR_BIT(_castle_rights, 0);
                    return *this;
                }

                castle(0b0011);
                // CLEAR_BIT(_castle_rights, 1); old
                // CLEAR_BIT(_castle_rights, 0);
            }
            if(getRookFlag(move))
            {
                // clear castling if rook moved from it's original square
                if(source_square == H8)
                {
                    castle(0b0010);
                    // CLEAR_BIT(_castle_rights, 1); old
                }
                else if(source_square == A8)
                {
                    castle(0b0001);
                    // CLEAR_BIT(_castle_rights, 0); old
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
        for(Piece enemy_piece : (next_side == Side::WHITE ? whitePieces : blackPieces) )
        {
            // if is nessecery because of hashing
            if(GET_BIT(_piece_bitboards[enemy_piece], target_square))
            {
                capturePiece(enemy_piece, target_square, next_side);
                // CLEAR_BIT(_piece_bitboards[enemy_piece], target_square); old
                // CLEAR_BIT(_occ_bitboards[next_side], target_square);
                break;
            }
        }
    }

    if(getEnPassantFlag(move))
    {
        if(next_side == Side::BLACK)
        {
            captureEnPassant(p, target_square-8, next_side);
            // CLEAR_BIT(_piece_bitboards[Piece::p], target_square-8); old
            // CLEAR_BIT(_occ_bitboards[next_side], target_square-8);
            // CLEAR_BIT(_occ_bitboards[Side::BOTH], target_square-8);
        }
        else
        {
            captureEnPassant(P, target_square+8, next_side);
            // CLEAR_BIT(_piece_bitboards[Piece::P], target_square+8); old
            // CLEAR_BIT(_occ_bitboards[next_side], target_square+8);
            // CLEAR_BIT(_occ_bitboards[Side::BOTH], target_square+8);
        }
    }

    Piece promotion_piece = getPromotionPiece(move);
    if(promotion_piece != Piece::no_piece)
    {
        if(next_side == Side::BLACK)
        {
            promote(P, promotion_piece, target_square);
            // CLEAR_BIT(_piece_bitboards[Piece::P], target_square); old
            // SET_BIT(_piece_bitboards[promotion_piece], target_square);
        }
        else
        {
            promote(p, promotion_piece, target_square);
            // CLEAR_BIT(_piece_bitboards[Piece::p], target_square); old
            // SET_BIT(_piece_bitboards[promotion_piece], target_square);
        }
    }

    /* Hash new en passant and side to move info  */
    _hash ^= side_hash_key ^ en_passant_hash_keys[_en_passant];

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
