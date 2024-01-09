#include "board_repr.h"
#include "../utils/attack_tables/attack_tables.h"

// flags should already be at proper bit indexes
#define MAKE_MOVE(flag, src_idx, target_idx) ((flag) | ((src_idx) << 6) | (target_idx))

_ForceInline U64 Board::get_white_pawn_attack(int idx)
{
    return white_pawn_attack[idx];
}

_ForceInline U64 Board::get_black_pawn_attack(int idx)
{
    return black_pawn_attack[idx];
}

_ForceInline U64 Board::get_knight_attack(int idx)
{
    return knight_attack[idx];
}

_ForceInline U64 Board::get_king_attack(int idx)
{
    return king_attack[idx];
}

_ForceInline U64 Board::get_bishop_attack(int idx)
{
    return _get_bishop_attack(_occ_bitboards[both], idx);
}

_ForceInline U64 Board::get_rook_attack(int idx)
{
    return _get_rook_attack(_occ_bitboards[both], idx);
}

_ForceInline U64 Board::get_bishop_checkmask(int idx, int king_idx)
{
    return bishop_checkmask[king_idx][idx];
}

_ForceInline U64 Board::get_rook_checkmask(int idx, int king_idx)
{
    return rook_checkmask[king_idx][idx];
}

_ForceInline U64 Board::get_queen_attack(int idx)
{
    return _get_bishop_attack(_occ_bitboards[both], idx) |
           _get_rook_attack(_occ_bitboards[both], idx);
}

/* -------------------------------------------------------------------------- */
/*                        CONSTEXPR APPLYING FUNCTIONS                        */
/* -------------------------------------------------------------------------- */

template<bool WhiteMove>
constexpr Piece enemyPiece(Piece piece) {
    if constexpr (WhiteMove)
    {
        return static_cast<Piece>( static_cast<int>(piece) + 6 );
    }
    else
    {
        return piece;
    }
}

template<bool WhiteMove>
constexpr Piece playerPiece(Piece piece) {
    if constexpr (WhiteMove)
    {
        return piece;
    }
    else
    {
        return static_cast<Piece>( static_cast<int>(piece) + 6 );
    }
}

template<bool WhiteMove>
constexpr Side playerSide()
{
    if constexpr(WhiteMove)
    {
        return Side::white;
    }
    else
    {
        return Side::black;
    }
}

template<bool WhiteMove>
constexpr Side enemySide()
{
    if constexpr(WhiteMove)
    {
        return Side::black;
    }
    else
    {
        return Side::white;
    }
}

template<bool WhiteMove>
constexpr U64 kingsideCastleMask()
{
    if constexpr (WhiteMove)
    {
        //return 0xEUL;
        return (1UL << E1) | (1UL << F1) | (1UL << G1);
    }
    else
    {
        //return 0xE0000000000000000UL;
        return (1UL << E8) | (1UL << F8) | (1UL << G8);
    }
}

template<bool WhiteMove>
constexpr U64 queensideCastleMask()
{
    if constexpr (WhiteMove)
    {
        return (1UL << E1) | (1UL << D1) | (1UL << C1) | (1UL << B1);
    }
    else
    {
        return (1UL << E8) | (1UL << D8) | (1UL << C8) | (1UL << B8);
    }
}

template<bool WhiteMove>
constexpr int kingsideCastleTarget()
{
    if constexpr (WhiteMove)
    {
        return G1;
    }
    else
    {
        return G8;
    }
}

template<bool WhiteMove>
constexpr int queensideCastleTarget()
{
    if constexpr (WhiteMove)
    {
        return C1;
    }
    else
    {
        return C8;
    }
}

/* -------------------------------------------------------------------------- */
/*                               MOVE GENERATION                              */
/* -------------------------------------------------------------------------- */

template<bool WhiteMove, bool ENPoss, bool Kcastle, bool Qcastle, bool kcastle, bool qcastle>
std::vector<U16> Board::_getMoves()
{

    /*
    Notes:
     - Due to WhiteMove constexpr every call relying on side to play information should
       be implemented as white but pass through appropriate constexpr functions.
     - To make it more transparent the player playing is called player and the player
       not playing is called enemy.
    
        example: call to _piece_bitboard[p] (previously ment getting black pawns now should be)
                         _piece_bitboard[enemyPiece<WhiteMove>(P)] (which will provid in constexpr
                                needed piece enum)

    */

    /* _bitboard(|2|3|4) are operational bitboards (like registers) */
    int idx, idx2, check_count=0, king_idx = bit_index(_piece_bitboards[playerPiece<WhiteMove>(K)]);
    U64 _bitboard, _bitboard2, _bitboard3, _bitboard4, _enemy_attack_bb=0, rook_pins=0, bishop_pins=0, checkmask=0, sadsauqre_bb=0, movemask;
    std::vector<U16> moves;

    /* -------------------------- ENEMY ATTACK MAP FILL ------------------------- */

    // line to include queens into bishop and rook evaluations
    _bitboard3 = _piece_bitboards[enemyPiece<WhiteMove>(Q)];

    // enemy bishop attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(B)] | _bitboard3;
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        _enemy_attack_bb |= get_bishop_attack(idx);
        CLEAR_BIT(_bitboard, idx);
        // -- pinmask and checkmask part
        _bitboard2 = get_bishop_checkmask(idx, king_idx);
        if(bit_count(_bitboard2 & _occ_bitboards[enemySide<WhiteMove>()]) == 1)
        {
            // needed condition for any pin or check to exist -> else there is none
            switch (bit_count(_bitboard2 & _occ_bitboards[playerSide<WhiteMove>()]))
            {
            case 0:
                /* this should be added to checkmask */
                checkmask |= _bitboard2;
                check_count++;
                break;

            case 1:
                /* this should be added to pinmaks */
                bishop_pins |= _bitboard2;
                break;
            
            default:
                /* more than 2 pieces in pin is not a pin */
                break;
            }
        }
        // -- end
    }

    //enemy rook attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(R)] | _bitboard3;
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        _enemy_attack_bb |= get_rook_attack(idx);
        CLEAR_BIT(_bitboard, idx);
        // -- pinmask and checkmask part
        _bitboard2 = get_rook_checkmask(idx, king_idx);
        if(bit_count(_bitboard2 & _occ_bitboards[enemySide<WhiteMove>()]) == 1)
        {
            // needed condition for any pin or check to exist -> else there is none
            switch (bit_count(_bitboard2 & _occ_bitboards[playerSide<WhiteMove>()]))
            {
            case 0:
                /* this should be added to checkmask */
                checkmask |= _bitboard2;
                check_count++;
                break;

            case 1:
                /* this should be added to pinmaks */
                rook_pins |= _bitboard2;
                break;
            
            default:
                /* more than 2 pieces in pin is not a pin */
                break;
            }
        }
        // -- end
    }

    // enemy knight attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(N)];
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        _bitboard3 = get_knight_attack(idx);
        _enemy_attack_bb |= _bitboard3;
        CLEAR_BIT(_bitboard, idx);
        // -- pinmask and checkmask part
        if(_piece_bitboards[playerPiece<WhiteMove>(K)] & _bitboard3)
        {
            SET_BIT(checkmask, idx);
            check_count++;
        }
        // -- end
    }

    // enemy king attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(K)];
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        _enemy_attack_bb |= get_king_attack(idx);
        CLEAR_BIT(_bitboard, idx);
    }

    // enemy pawn attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(P)];
    while(_bitboard)
    {
        if constexpr (WhiteMove)
        {
            idx = bitScanForward(_bitboard);
            _bitboard2 = get_black_pawn_attack(idx);
            _enemy_attack_bb |= _bitboard2;
            CLEAR_BIT(_bitboard, idx);

            // if or to_1_shift faster??
            check_count += to_1_shift(_bitboard2 & _piece_bitboards[playerPiece<WhiteMove>(K)]);
        }
        else
        {
            idx = bitScanForward(_bitboard);
            _bitboard2 = get_white_pawn_attack(idx);
            _enemy_attack_bb |= _bitboard2;
            CLEAR_BIT(_bitboard, idx);

            // if or to_1_shift faster??
            check_count += to_1_shift(_bitboard2 & _piece_bitboards[playerPiece<WhiteMove>(K)]);
        }
    }

    //Divide into position with one or less checks and positions with more checks
    switch (check_count)
    {
    case 0:
        sadsauqre_bb = ~_get_sadsquare(king_idx, checkmask);
        checkmask = UINT64_MAX;
        // break is ommited here not by accident

    case 1:
        movemask = ~(_occ_bitboards[playerSide<WhiteMove>()]) & checkmask;

        /* ----------------------------- STANDARD MOVES ----------------------------- */

        //knight eval
        _bitboard = _piece_bitboards[playerPiece<WhiteMove>(N)] & ~(rook_pins | bishop_pins); // prefilter pinned knights
        while(_bitboard)
        {
            idx = bitScanForward(_bitboard);
            CLEAR_BIT(_bitboard, idx);

            _bitboard2 = get_knight_attack(idx) & movemask;
            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        //bishop eval
        _bitboard = _piece_bitboards[playerPiece<WhiteMove>(B)] & ~rook_pins; // prefilter bishops pinned by rooks

        // iterate through pinned bishops
        _bitboard3 = (_bitboard | _piece_bitboards[playerPiece<WhiteMove>(Q)]) & bishop_pins;   // diagonaly pinned queens are an addition here
        while(_bitboard3)
        {
            idx = bitScanForward(_bitboard3);
            CLEAR_BIT(_bitboard3, idx);

            _bitboard2 = get_bishop_attack(idx) & movemask & bishop_pins;
            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        // iterate through not pinned bishops
        _bitboard3 = _bitboard & ~bishop_pins;
        while(_bitboard3)
        {
            idx = bitScanForward(_bitboard3);
            CLEAR_BIT(_bitboard3, idx);

            _bitboard2 = get_bishop_attack(idx) & movemask;
            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        //rook eval
        _bitboard = _piece_bitboards[playerPiece<WhiteMove>(R)] & ~bishop_pins; // prefilter bishops pinned by bishops

        // iterate through pinned rooks
        _bitboard3 = (_bitboard | _piece_bitboards[playerPiece<WhiteMove>(Q)]) & rook_pins; // rook pinned queens are also evaluated here
        while(_bitboard3)
        {
            idx = bitScanForward(_bitboard3);
            CLEAR_BIT(_bitboard3, idx);

            _bitboard2 = get_rook_attack(idx) & movemask & rook_pins;
            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        // iterate through not pinned rooks
        _bitboard3 = _bitboard & ~rook_pins;
        while(_bitboard3)
        {
            idx = bitScanForward(_bitboard3);
            CLEAR_BIT(_bitboard3, idx);

            _bitboard2 = get_rook_attack(idx) & movemask;
            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        // queen eval
        _bitboard = _piece_bitboards[playerPiece<WhiteMove>(Q)] & ~(bishop_pins | rook_pins); // only not pinned queens are evaluated here
        while(_bitboard)
        {
            idx = bitScanForward(_bitboard);
            CLEAR_BIT(_bitboard, idx);

            _bitboard2 = get_queen_attack(idx) &movemask;
            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        /* -------------------------------- PAWNMOVES ------------------------------- */

        _bitboard = _piece_bitboards[playerPiece<WhiteMove>(P)];
        _bitboard2 = _bitboard & rook_pins; // pawns pined only by rooks
        _bitboard3 = _bitboard & bishop_pins; // pawns pinned by bishops
        _bitboard4 = _bitboard & ~(rook_pins | bishop_pins); //not pinned pawns;

        U64 forward_pawn_moves;

        //farward move for all
        if constexpr (WhiteMove)
        {
            forward_pawn_moves = (((_bitboard & ~(_occ_bitboards[enemySide<WhiteMove>()] >> 8)) << 8) |
                  ((_bitboard & ROW_2 & ~(_occ_bitboards[enemySide<WhiteMove>()] >> 16)) << 16)) & movemask;
        }
        else
        {
            forward_pawn_moves = (((_bitboard & ~(_occ_bitboards[enemySide<WhiteMove>()] << 8)) >> 8) |
                  ((_bitboard & ROW_2 & ~(_occ_bitboards[enemySide<WhiteMove>()] << 16)) >> 16)) & movemask;
        }

        while(_bitboard2)
        {
            idx = bitScanForward(_bitboard2);
            CLEAR_BIT(_bitboard2, idx);

            _bitboard = forward_pawn_moves & cols_get[idx % 8] & rook_pins;
            while(_bitboard)
            {
                idx2 = bitScanForward(_bitboard);
                CLEAR_BIT(_bitboard, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        while(_bitboard3)
        {
            idx = bitScanForward(_bitboard3);
            CLEAR_BIT(_bitboard3, idx);

            if constexpr (WhiteMove)
            {
                _bitboard = get_white_pawn_attack(idx) & _occ_bitboards[enemySide<WhiteMove>()] & bishop_pins & checkmask;
            }
            else
            {
                _bitboard = get_black_pawn_attack(idx) & _occ_bitboards[enemySide<WhiteMove>()] & bishop_pins & checkmask;
            }

            while(_bitboard)
            {
                idx2 = bitScanForward(_bitboard);
                CLEAR_BIT(_bitboard, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        while(_bitboard4)
        {
            idx = bitScanForward(_bitboard4);
            CLEAR_BIT(_bitboard4, idx);

            if constexpr (WhiteMove)
            {
                _bitboard2 = get_white_pawn_attack(idx) & _occ_bitboards[enemySide<WhiteMove>()] & checkmask;
            }
            else
            {
                _bitboard2 = get_black_pawn_attack(idx) & _occ_bitboards[enemySide<WhiteMove>()] & checkmask;
            }

            _bitboard = forward_pawn_moves & cols_get[idx % 8];
            while(_bitboard)
            {
                idx2 = bitScanForward(_bitboard);
                CLEAR_BIT(_bitboard, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }

            while(_bitboard2)
            {
                idx2 = bitScanForward(_bitboard2);
                CLEAR_BIT(_bitboard2, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        /* ------------------------------- KING MOVES ------------------------------- */

        _bitboard = get_king_attack(king_idx) & ~_enemy_attack_bb & ~_occ_bitboards[playerSide<WhiteMove>()] & sadsauqre_bb;

        while(_bitboard)
        {
            idx2 = bitScanForward(_bitboard);
            CLEAR_BIT(_bitboard, idx2);
            moves.push_back(MAKE_MOVE(0, king_idx, idx2));
        }

        /* If a move that makes the  */
        if constexpr ((WhiteMove && Kcastle) || (!WhiteMove && kcastle))
        {
            // player kingside castle possible
            if( !(kingsideCastleMask<WhiteMove>() & (_enemy_attack_bb | (_occ_bitboards[playerSide<WhiteMove>()] & ~_piece_bitboards[playerPiece<WhiteMove>(K)]))) )
            {
                moves.push_back(MAKE_MOVE(0b0011000000000000, king_idx, kingsideCastleTarget<WhiteMove>()));
            }
        }
        if constexpr ((WhiteMove && Qcastle) || (!WhiteMove && qcastle))
        {
            // player queenside castle possible
            if( !(queensideCastleMask<WhiteMove>() & (_enemy_attack_bb | (_occ_bitboards[playerSide<WhiteMove>()] & ~_piece_bitboards[playerPiece<WhiteMove>(K)]))) )
            {
                moves.push_back(MAKE_MOVE(0b0011000000000000, king_idx, queensideCastleTarget<WhiteMove>()));
            }
        }

        break;
    
    default:
        /* only king moves (no castling) */
        _bitboard = get_king_attack(king_idx) & ~_enemy_attack_bb & ~_occ_bitboards[playerSide<WhiteMove>()] & ~_get_sadsquare(king_idx, checkmask);

        while(_bitboard)
        {
            idx2 = bitScanForward(_bitboard);
            CLEAR_BIT(_bitboard, idx2);
            moves.push_back(MAKE_MOVE(0, king_idx, idx2));
        }
    }

    return moves;
}



/* -------------------------------------------------------------------------- */
/*                       MOVE GENERATION TEMPLATE WRAPER                      */
/* -------------------------------------------------------------------------- */

std::vector<U16> Board::getMoves()
{

    // consider changing it to multidimentional lookup array(if the array would work faster there are possibilities to increase the number of parameters)

    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 15) {return _getMoves<true, false, true, true, true, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 14) {return _getMoves<true, false, true, true, true, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 13) {return _getMoves<true, false, true, true, false, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 12) {return _getMoves<true, false, true, true, false, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 11) {return _getMoves<true, false, true, false, true, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 10) {return _getMoves<true, false, true, false, true, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 9) {return _getMoves<true, false, true, false, false, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 8) {return _getMoves<true, false, true, false, false, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 7) {return _getMoves<true, false, false, true, true, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 6) {return _getMoves<true, false, false, true, true, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 5) {return _getMoves<true, false, false, true, false, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 4) {return _getMoves<true, false, false, true, false, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 3) {return _getMoves<true, false, false, false, true, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 2) {return _getMoves<true, false, false, false, true, false>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 1) {return _getMoves<true, false, false, false, false, true>();}
    if(_side_to_move == Side::white && _en_passant == NO_SQ && _castle_rights == 0) {return _getMoves<true, false, false, false, false, false>();}

    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 15) {return _getMoves<true, true, true, true, true, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 14) {return _getMoves<true, true, true, true, true, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 13) {return _getMoves<true, true, true, true, false, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 12) {return _getMoves<true, true, true, true, false, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 11) {return _getMoves<true, true, true, false, true, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 10) {return _getMoves<true, true, true, false, true, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 9) {return _getMoves<true, true, true, false, false, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 8) {return _getMoves<true, true, true, false, false, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 7) {return _getMoves<true, true, false, true, true, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 6) {return _getMoves<true, true, false, true, true, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 5) {return _getMoves<true, true, false, true, false, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 4) {return _getMoves<true, true, false, true, false, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 3) {return _getMoves<true, true, false, false, true, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 2) {return _getMoves<true, true, false, false, true, false>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 1) {return _getMoves<true, true, false, false, false, true>();}
    if(_side_to_move == Side::white && _en_passant != NO_SQ && _castle_rights == 0) {return _getMoves<true, true, false, false, false, false>();}

    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 15) {return _getMoves<false, false, true, true, true, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 14) {return _getMoves<false, false, true, true, true, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 13) {return _getMoves<false, false, true, true, false, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 12) {return _getMoves<false, false, true, true, false, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 11) {return _getMoves<false, false, true, false, true, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 10) {return _getMoves<false, false, true, false, true, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 9) {return _getMoves<false, false, true, false, false, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 8) {return _getMoves<false, false, true, false, false, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 7) {return _getMoves<false, false, false, true, true, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 6) {return _getMoves<false, false, false, true, true, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 5) {return _getMoves<false, false, false, true, false, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 4) {return _getMoves<false, false, false, true, false, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 3) {return _getMoves<false, false, false, false, true, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 2) {return _getMoves<false, false, false, false, true, false>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 1) {return _getMoves<false, false, false, false, false, true>();}
    if(_side_to_move == Side::black && _en_passant == NO_SQ && _castle_rights == 0) {return _getMoves<false, false, false, false, false, false>();}

    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 15) {return _getMoves<false, true, true, true, true, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 14) {return _getMoves<false, true, true, true, true, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 13) {return _getMoves<false, true, true, true, false, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 12) {return _getMoves<false, true, true, true, false, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 11) {return _getMoves<false, true, true, false, true, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 10) {return _getMoves<false, true, true, false, true, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 9) {return _getMoves<false, true, true, false, false, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 8) {return _getMoves<false, true, true, false, false, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 7) {return _getMoves<false, true, false, true, true, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 6) {return _getMoves<false, true, false, true, true, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 5) {return _getMoves<false, true, false, true, false, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 4) {return _getMoves<false, true, false, true, false, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 3) {return _getMoves<false, true, false, false, true, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 2) {return _getMoves<false, true, false, false, true, false>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 1) {return _getMoves<false, true, false, false, false, true>();}
    if(_side_to_move == Side::black && _en_passant != NO_SQ && _castle_rights == 0) {return _getMoves<false, true, false, false, false, false>();}
    return std::vector<U16>();
}
