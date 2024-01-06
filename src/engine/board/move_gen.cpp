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

    /* _bitboard and bitboard to are operational bitboards */
    int idx, idx2, check_count=0;
    int king_idx = bit_index(_piece_bitboards[playerPiece<WhiteMove>(K)]);
    U64 _bitboard, _bitboard2, _bitboard3, _bitboard4, _enemy_attack_bb=0, rook_pins=0, bishop_pins=0, checkmask=0, movemask;
    U64 attacks[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<U16> moves;

    /*
    
    Idea:

     + Generate pieces attacks for all the pieces
     + Generate not side to move attack map
     
     + Generate pawn pushes
     + Generate Castles (based on not side to move attack map)
     + Generate en passant

     + Generate pin mask
     + Generate check mask

     + filter all the moves generated till now with pin masks and check masks
        (normal attack map moves, pawn pushes, en passant (everything))

     + Iterate moves and return with flags(like [quiet move, capture, double pawn push, castle, kingmove, rookmove]).

     !! Two bit scans? one at the start to generate pseudo legal moves and after filtering the seccon at the end to
        get proper move format an filter which piece accualy done the move?

    */

    // enemy attack map fillup


    /*
    IDEA:
    Embed the pinmask and checkmas creation into the attacks lookup for all black pieces to save bitscans cycles.
    The pieces need to be evaluated one by one anyway soo this souds pretty comfy.
    */

    // enemy bishop attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(B)];
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        attacks[enemyPiece<WhiteMove>(B)] |= get_bishop_attack(idx);
        _enemy_attack_bb |= attacks[enemyPiece<WhiteMove>(B)];
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
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(R)];
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        attacks[enemyPiece<WhiteMove>(R)] |= get_rook_attack(idx);
        _enemy_attack_bb |= attacks[enemyPiece<WhiteMove>(R)];
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
        attacks[enemyPiece<WhiteMove>(N)] |= get_knight_attack(idx);
        _enemy_attack_bb |= attacks[enemyPiece<WhiteMove>(N)];
        CLEAR_BIT(_bitboard, idx);
        // -- pinmask and checkmask part
        _bitboard2 = get_rook_checkmask(idx, king_idx);
        if(_piece_bitboards[playerPiece<WhiteMove>(K)] & attacks[enemyPiece<WhiteMove>(N)])
        {
            SET_BIT(checkmask, idx);
            check_count++;
        }
        // -- end
    }

    // enemy queen attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(Q)];
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        attacks[enemyPiece<WhiteMove>(Q)] |= get_queen_attack(idx);
        _enemy_attack_bb |= attacks[enemyPiece<WhiteMove>(B)];
        CLEAR_BIT(_bitboard, idx);

        // -- pinmask and checkmask part
        // first bishoplike attacks
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
        else // there is bishoplike move of queen it can no longer provide any rooklike move
        {
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
        }
        // -- end
    }

    // enemy king attack processing
    _bitboard = _piece_bitboards[enemyPiece<WhiteMove>(K)];
    while(_bitboard)
    {
        idx = bitScanForward(_bitboard);
        attacks[enemyPiece<WhiteMove>(K)] |= get_king_attack(idx);
        _enemy_attack_bb |= attacks[enemyPiece<WhiteMove>(K)];
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
            attacks[enemyPiece<WhiteMove>(P)] |= _bitboard2;
            _enemy_attack_bb |= _bitboard2;
            CLEAR_BIT(_bitboard, idx);

            // if or to_1_shift faster??
            check_count += to_1_shift(_bitboard2 & _piece_bitboards[playerPiece<WhiteMove>(K)]);
        }
        else
        {
            idx = bitScanForward(_bitboard);
            _bitboard2 = get_white_pawn_attack(idx);
            attacks[enemyPiece<WhiteMove>(P)] |= _bitboard2;
            _enemy_attack_bb |= _bitboard2;
            CLEAR_BIT(_bitboard, idx);

            // if or to_1_shift faster??
            check_count += to_1_shift(_bitboard2 & _piece_bitboards[playerPiece<WhiteMove>(K)]);
        }
    }

    //Divide into position with one or less checks and positions with more checks
    if(check_count <= 1)
    {
        // create movemask
        if(!checkmask)
        {
            checkmask = UINT64_MAX;
        }
        movemask = ~(_occ_bitboards[playerSide<WhiteMove>()]) & checkmask;

        /*
        IDEA:
        Because we need to bitscan the white pieces anyway (to generate intiger like moves),
        use it to simultaniously filter incoming moves and output them instatnly. Because of 
        that approach the problem of deciding which piece is responsible for what move no longer
        exists. Moreover, because we know the piece type being filtered, the problem of deciding
        what pinmask to use also dissapears.
        The only problem are pawns because shifting them one by one is not efficient.

        SADSQUARES IDEA:
        The idea:
            "
            Nor check mask or attack maps preven king from going backward
            SOLUTION:
            Create SADSQUARES lookup evaluated with pinmask and checkmask. If a slider piece attacks
            one of the king. We also know which square near king got attacked so we create map of squares
            that will be attacked if the king would move. For example rook attacks king from the right
            .....    ......                                   ...
            .K..R -> .K11R. Than the sad squares map would be 111 
            .....    ......                                   ...
            So this are the squares of original attack and attacks if king moves. Soo after the evaluation
            we get a sadsquares map which we simply logicaly NAND with kingmoves.
            "
        
        in case 0 there are no direct checks soo this doesn't need to be calculated
        in case 1 and default this applies. Because of how the checkmask is derived from pinmask we can 
        easily figure out where is the backsquare that king cannot move to.
            Explanation. We need to take only slider pieces into consideration because if a knight, or pawn
            attacks the king there is no trace to follow.
        The only problem with this approach is when a slider piece stands exactly next to the king. The sad
        squares will work correctly and prevent the king from moving backward but at the same time they don't
        allow the king to capture the checking piece. (This is a special case and needs to be handled separately
        for now)
        */

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
        _bitboard2 = _bitboard & rook_pins; // pawns pined only by
        _bitboard3 = _bitboard & bishop_pins; // pawns pinned by bishops
        _bitboard4 = _bitboard & ~(rook_pins | bishop_pins); //not pinned pawns;

        // this can be optimized (maybe consider puting this inside the while??? how often two> pawns are pinned by the rooks anyway?)
        // provide all pawn moves and store in _bitboard(rookpinned moves)
        if constexpr (WhiteMove)
        {
            // simple one step forward
            _bitboard = ((_bitboard2 & ~(_occ_bitboards[enemySide<WhiteMove>()] >> 8)) << 8) & movemask & rook_pins;
            // add seccond step forward for pawns that meet the criteria
            _bitboard |= ((_bitboard & ROW_3 & ~(_occ_bitboards[enemySide<WhiteMove>()] >> 8)) << 8) & movemask; // is & rook_pins needed here?
        }
        else
        {
            // simple one step forward
            _bitboard = ((_bitboard2 & ~(_occ_bitboards[enemySide<WhiteMove>()] << 8)) >> 8) & movemask & rook_pins;
            // add seccond step forward for pawns that meet the criteria
            _bitboard |= ((_bitboard & ROW_6 & ~(_occ_bitboards[enemySide<WhiteMove>()] << 8)) >> 8) & movemask; // is & rook_pins needed here?
        }

        while(_bitboard2)
        {
            idx = bitScanForward(_bitboard2);
            CLEAR_BIT(_bitboard2, idx);

            while(_bitboard & cols_get[idx % 8])
            {
                idx2 = bitScanForward(_bitboard & cols_get[idx % 8]);
                CLEAR_BIT(_bitboard, idx2);
                moves.push_back(MAKE_MOVE(0, idx, idx2));
            }
        }

        // this can be optimized
        // provide all pawn attacks, store in _bitboard/_bitboard2

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

        // standard no pinned pawns pushes
        if constexpr (WhiteMove)
        {
            // simple one step forward
            _bitboard = ((_bitboard4 & ~(_occ_bitboards[enemySide<WhiteMove>()] >> 8)) << 8) & movemask;
            // add seccond step forward for pawns that meet the criteria
            _bitboard |= ((_bitboard & ROW_3 & ~(_occ_bitboards[enemySide<WhiteMove>()] >> 8)) << 8) & movemask; // is & rook_pins needed here?
        }
        else
        {
            // simple one step forward
            _bitboard = ((_bitboard4 & ~(_occ_bitboards[enemySide<WhiteMove>()] << 8)) >> 8) & movemask;
            // add seccond step forward for pawns that meet the criteria
            _bitboard |= ((_bitboard & ROW_6 & ~(_occ_bitboards[enemySide<WhiteMove>()] << 8)) >> 8) & movemask; // is & rook_pins needed here?
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

            while(_bitboard & cols_get[idx % 8])
            {
                idx2 = bitScanForward(_bitboard & cols_get[idx % 8]);
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
    }
    else
    {
        // only king moves
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
