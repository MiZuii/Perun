#include "board_repr.h"

/* -------------------------------------------------------------------------- */
/*                        CONSTEXPR APPLYING FUNCTIONS                        */
/* -------------------------------------------------------------------------- */

template<bool WhiteMove>
_ForceInline constexpr U64 kingsideCastleMask()
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
_ForceInline constexpr U64 queensideCastleMask()
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
_ForceInline constexpr int kingsideCastleTarget()
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
_ForceInline constexpr int queensideCastleTarget()
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

template<bool WhiteMove>
_ForceInline bool enPassantPossible(U64 pawns_map, bool check_cond)
{
    if constexpr(WhiteMove)
    {
        return (ROW_5 & pawns_map) && check_cond;
    }
    else
    {
        return (ROW_4 & pawns_map) && check_cond;
    }
}

template<bool WhiteMove>
bool Board::_valid_en_passant()
{
    U64 bishops = _piece_bitboards[playerPiece<WhiteMove>(BISHOP)] | _piece_bitboards[playerPiece<WhiteMove>(QUEEN)];  
    U64 rooks = _piece_bitboards[playerPiece<WhiteMove>(ROOK)] | _piece_bitboards[playerPiece<WhiteMove>(QUEEN)];
    U64 king = _piece_bitboards[playerPiece<WhiteMove>(KING)];

    bitScan(bishops)
    {
        int idx = bit_index(bishops);
        if(get_bishop_attack(idx) & king)
        {
            return false;
        }
    }

    bitScan(rooks)
    {
        int idx = bit_index(rooks);
        if(get_rook_attack(idx) & king)
        {
            return false;
        }
    }

    return true;
}

template<bool WhiteMove>
_Inline void pushPawnMoves(std::vector<Move_t> &moves, int src_idx, int trg_idx, bool capture_flag)
{
    if constexpr (WhiteMove)
    {
        if(trg_idx/8 == _8)
        {
            // promotion possible (white)
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(QUEEN), capture_flag, false, false, false, false, true));
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(ROOK), capture_flag, false, false, false, false, true));
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(BISHOP), capture_flag, false, false, false, false, true));
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(KNIGHT), capture_flag, false, false, false, false, true));
        }
        else
        {
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), no_piece, capture_flag, trg_idx-src_idx == 16, false, false, false, true));
        }
    }
    else
    {
        if(trg_idx/8 == _1)
        {
            // promotion possible (black)
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(QUEEN), capture_flag, false, false, false, false, true));
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(ROOK), capture_flag, false, false, false, false, true));
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(BISHOP), capture_flag, false, false, false, false, true));
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), playerPiece<WhiteMove>(KNIGHT), capture_flag, false, false, false, false, true));
        }
        else
        {
            moves.push_back(createMove(src_idx, trg_idx, playerPiece<WhiteMove>(PAWN), no_piece, capture_flag, src_idx-trg_idx == 16, false, false, false, true));
        }
    }
}

/* -------------------------------------------------------------------------- */
/*                                BOARD REFRESH                               */
/* -------------------------------------------------------------------------- */

template<bool WhiteMove>
void Board::_register_rook_pins()
{
    U64 pinners = get_rook_attack_pin(king_idx) & (_piece_bitboards[enemyPiece<WhiteMove>(ROOK)] | _piece_bitboards[enemyPiece<WhiteMove>(QUEEN)]); // xraymask here -> bitscan works as an if
    bitScan(pinners)
    {
        int pinner_idx = bit_index(pinners);
        U64 pinmask = get_rook_checkmask(pinner_idx, king_idx);
        if(pinmask & _occ_bitboards[playerSide<WhiteMove>()])
        {
            _rook_pins |= pinmask;
        }
    }
}

template<bool WhiteMove>
void Board::_register_bishop_pins()
{
    U64 pinners = get_bishop_attack_pin(king_idx) & (_piece_bitboards[enemyPiece<WhiteMove>(BISHOP)] | _piece_bitboards[enemyPiece<WhiteMove>(QUEEN)]); // xraymask here
    bitScan(pinners)
    {
        int pinner_idx = bit_index(pinners);
        U64 pinmask = get_bishop_checkmask(pinner_idx, king_idx);
        if(pinmask & _occ_bitboards[playerSide<WhiteMove>()])
        {
            _bishop_pins |= pinmask;
        }
    }
}

template<bool WhiteMove>
void Board::_refresh()
{
    U64 king_bitboard = _piece_bitboards[playerPiece<WhiteMove>(KING)];
    king_idx = bit_index(king_bitboard);

    /* -------------------------- ENEMY ATTACK MAP FILL ------------------------- */
    {
        const U64 enemy_queen = _piece_bitboards[enemyPiece<WhiteMove>(QUEEN)];


        // enemy bishop attack processing
        {

            U64 enemy_bishops = _piece_bitboards[enemyPiece<WhiteMove>(BISHOP)] | enemy_queen;
            bitScan(enemy_bishops)
            {
                int bishop_index = bit_index(enemy_bishops);
                U64 bishop_attack = get_bishop_attack(bishop_index);
                _enemy_attack_bb |= bishop_attack;
                _checkers |= king_bitboard & bishop_attack ? (1UL << bishop_index) : 0;
            }
        }


        //enemy rook attack processing
        {
            U64 enemy_rooks = _piece_bitboards[enemyPiece<WhiteMove>(ROOK)] | enemy_queen;
            bitScan(enemy_rooks)
            {
                int rook_index = bit_index(enemy_rooks);
                U64 rook_attack = get_rook_attack(rook_index);
                _enemy_attack_bb |= rook_attack;
                _checkers |= king_bitboard & rook_attack ? (1UL << rook_index) : 0;
            }
        }


        // enemy knight attack processing
        {
            U64 enemy_knights = _piece_bitboards[enemyPiece<WhiteMove>(KNIGHT)];
            bitScan(enemy_knights)
            {
                int knight_index = bit_index(enemy_knights);
                U64 knight_attack = get_knight_attack(knight_index);
                _enemy_attack_bb |= knight_attack;
                _checkers |= king_bitboard & knight_attack ? (1UL << knight_index) : 0;
            }
        }

        // enemy king attack processing
        {
            U64 enemy_king = _piece_bitboards[enemyPiece<WhiteMove>(KING)];
            bitScan(enemy_king)
            {
                _enemy_attack_bb |= get_king_attack(bit_index(enemy_king));
            }
        }

        // consider moving this to the from and checking if any pawn checks the king
        // by the enemy_attack_bb. Because only one pawn can attack at once the checker
        // bitboard can be updated by one logic formula instead repeating in a loop
        // enemy pawn attack processing
        {
            U64 enemy_pawns = _piece_bitboards[enemyPiece<WhiteMove>(PAWN)];
            bitScan(enemy_pawns)
            {
                if constexpr (WhiteMove)
                {
                    int pawn_index = bit_index(enemy_pawns);
                    U64 current_pawn_attack = get_black_pawn_attack(pawn_index);
                    _enemy_attack_bb |= current_pawn_attack;
                    _checkers |= king_bitboard & current_pawn_attack ? (1UL << pawn_index) : 0;
                }
                else
                {
                    int pawn_index = bit_index(enemy_pawns);
                    U64 current_pawn_attack = get_white_pawn_attack(pawn_index);
                    _enemy_attack_bb |= current_pawn_attack;
                    _checkers |= king_bitboard & current_pawn_attack ? (1UL << pawn_index) : 0;
                }
            }
        }

        // fill checkmask
        {
            U64 bs_checkers = _checkers;
            bitScan(bs_checkers)
            {
                const int checker_index = bit_index(bs_checkers);
                const U64 checker_bitboard = (1UL << checker_index);
                if(checker_bitboard & (_piece_bitboards[enemyPiece<WhiteMove>(BISHOP)] | enemy_queen))
                {
                    // register bishoplike check
                    _checkmask |= get_bishop_checkmask(checker_index, king_idx);
                }
                if(checker_bitboard & (_piece_bitboards[enemyPiece<WhiteMove>(ROOK)] | enemy_queen))
                {
                    // register rooklike check
                    _checkmask |= get_rook_checkmask(checker_index, king_idx);
                }
            }
        }

        // create pinmasks
        {
            _register_rook_pins<WhiteMove>();
            _register_bishop_pins<WhiteMove>();
        }
    }
}

/* -------------------------------------------------------------------------- */
/*                               MOVE GENERATION                              */
/* -------------------------------------------------------------------------- */

template<bool WhiteMove, bool ENPoss, bool Kcastle, bool Qcastle, bool kcastle, bool qcastle>
void Board::_getMoves()
{

    _refresh<WhiteMove>();

    if(bit_count(_checkers) != 2)
    {
        U64 sadsquare_bb    = ~_get_sadsquare(king_idx, _checkmask);
        _checkmask          = _checkers ? (_checkmask | _checkers) : UINT64_MAX;
        U64 movemask        = ~(_occ_bitboards[playerSide<WhiteMove>()]) & _checkmask;

        /* ----------------------------- STANDARD MOVES ----------------------------- */

        //knight eval
        {
            U64 player_knights = _piece_bitboards[playerPiece<WhiteMove>(KNIGHT)] & ~(_rook_pins | _bishop_pins); // prefilter pinned knights
            bitScan(player_knights)
            {
                int knight_idx = bit_index(player_knights);

                U64 legal_knight_moves = get_knight_attack(knight_idx) & movemask;
                bitScan(legal_knight_moves)
                {
                    int legal_move_idx = bit_index(legal_knight_moves);
                    moves.push_back(createMove(knight_idx, legal_move_idx, playerPiece<WhiteMove>(KNIGHT), no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, false, false, false));
                }
            }
        }


        //bishop eval
        {
            U64 player_bishops = _piece_bitboards[playerPiece<WhiteMove>(BISHOP)] & ~_rook_pins;

            // iterate through pinned bishops
            {
                U64 pinned_bishops = (player_bishops | _piece_bitboards[playerPiece<WhiteMove>(QUEEN)]) & _bishop_pins;   // diagonaly pinned queens are an addition here
                bitScan(pinned_bishops)
                {
                    int bishop_idx = bit_index(pinned_bishops);
                    U64 legal_bishop_moves = get_bishop_attack(bishop_idx) & _bishop_pins & movemask;

                    bitScan(legal_bishop_moves)
                    {
                        int legal_move_idx = bit_index(legal_bishop_moves);
                        moves.push_back(createMove(bishop_idx, legal_move_idx, 
                            player_bishops & (1UL << bishop_idx) ? playerPiece<WhiteMove>(BISHOP) : playerPiece<WhiteMove>(QUEEN), 
                            no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, false, false, false));
                    }
                }
            }

            // iterate through not pinned bishops
            {
                U64 not_pinned_bishops = player_bishops & ~_bishop_pins;
                bitScan(not_pinned_bishops)
                {
                    int bishop_idx = bit_index(not_pinned_bishops);
                    U64 legal_bishop_moves = get_bishop_attack(bishop_idx) & movemask;

                    bitScan(legal_bishop_moves)
                    {
                        int legal_move_idx = bit_index(legal_bishop_moves);
                        moves.push_back(createMove(bishop_idx, legal_move_idx, 
                            player_bishops & (1UL << bishop_idx) ? playerPiece<WhiteMove>(BISHOP) : playerPiece<WhiteMove>(QUEEN), 
                            no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, false, false, false));
                    }
                }
            }
        }


        //rook eval
        {
            U64 player_rooks = _piece_bitboards[playerPiece<WhiteMove>(ROOK)] & ~_bishop_pins;

            // iterate through pinned rooks
            {
                U64 pinned_rooks = (player_rooks | _piece_bitboards[playerPiece<WhiteMove>(QUEEN)]) & _rook_pins;
                bitScan(pinned_rooks)
                {
                    int rook_idx = bit_index(pinned_rooks);
                    U64 legal_rook_moves = get_rook_attack(rook_idx) & _rook_pins & movemask;

                    bitScan(legal_rook_moves)
                    {
                        int legal_move_idx = bit_index(legal_rook_moves);
                        moves.push_back(createMove(rook_idx, legal_move_idx, 
                            player_rooks & (1UL << rook_idx) ? playerPiece<WhiteMove>(ROOK) : playerPiece<WhiteMove>(QUEEN), 
                            no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, false, true, false));
                    }
                }
            }

            // iterate through not pinned rooks
            {
                U64 not_pinned_rooks = player_rooks & ~_rook_pins;
                bitScan(not_pinned_rooks)
                {
                    int rook_idx = bit_index(not_pinned_rooks);
                    U64 legal_rook_moves = get_rook_attack(rook_idx) & movemask;

                    bitScan(legal_rook_moves)
                    {
                        int legal_move_idx = bit_index(legal_rook_moves);
                        moves.push_back(createMove(rook_idx, legal_move_idx, 
                            player_rooks & (1UL << rook_idx) ? playerPiece<WhiteMove>(ROOK) : playerPiece<WhiteMove>(QUEEN),
                            no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, false, true, false));
                    }
                }
            }
        }

        // queen eval
        {
            U64 player_queen = _piece_bitboards[playerPiece<WhiteMove>(QUEEN)] & ~(_bishop_pins | _rook_pins); // only not pinned queens are evaluated here
            bitScan(player_queen)
            {
                int queen_idx = bit_index(player_queen);
                U64 current_queen_attack = get_queen_attack(queen_idx) & movemask;

                bitScan(current_queen_attack)
                {
                    int legal_move_idx = bit_index(current_queen_attack);
                    moves.push_back(createMove(queen_idx, legal_move_idx, playerPiece<WhiteMove>(QUEEN), no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, false, false, false));
                }
            }
        }

        /* -------------------------------- PAWNMOVES ------------------------------- */

        {
            U64 player_pawns = _piece_bitboards[playerPiece<WhiteMove>(PAWN)];
            U64 forward_pawn_moves;

            //farward move for all
            if constexpr (WhiteMove)
            {
                forward_pawn_moves = (((player_pawns & ~(_occ_bitboards[Side::BOTH] >> 8)) << 8) |
                    ((player_pawns & ROW_2 & ~((_occ_bitboards[Side::BOTH] >> 16) | (_occ_bitboards[Side::BOTH] >> 8))) << 16)) & movemask;
            }
            else
            {
                forward_pawn_moves = (((player_pawns & ~(_occ_bitboards[Side::BOTH] << 8)) >> 8) |
                    ((player_pawns & ROW_7 & ~((_occ_bitboards[Side::BOTH] << 16) | (_occ_bitboards[Side::BOTH] << 8))) >> 16)) & movemask;
            }

            // only pawns pinned by rooks
            {
                U64 current_pawns = player_pawns & _rook_pins;
                bitScan(current_pawns)
                {
                    int pawn_idx = bit_index(current_pawns);
                    U64 legal_pawn_moves = forward_pawn_moves & cols_get[pawn_idx % 8] & _rook_pins;

                    bitScan(legal_pawn_moves)
                    {
                        int legal_move_idx = bit_index(legal_pawn_moves);
                        pushPawnMoves<WhiteMove>(moves, pawn_idx, legal_move_idx, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx));
                    }
                }
            }

            {
                U64 current_pawns = player_pawns & _bishop_pins;
                bitScan(current_pawns)
                {
                    int pawn_idx = bit_index(current_pawns);

                    U64 legal_pawn_moves;
                    if constexpr (WhiteMove)
                    {
                        legal_pawn_moves = get_white_pawn_attack(pawn_idx) & _occ_bitboards[enemySide<WhiteMove>()] & _bishop_pins & _checkmask;
                    }
                    else
                    {
                        legal_pawn_moves = get_black_pawn_attack(pawn_idx) & _occ_bitboards[enemySide<WhiteMove>()] & _bishop_pins & _checkmask;
                    }

                    bitScan(legal_pawn_moves)
                    {
                        int legal_move_idx = bit_index(legal_pawn_moves);
                        pushPawnMoves<WhiteMove>(moves, pawn_idx, legal_move_idx, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx));
                    }
                }
            }

            {
                U64 current_pawns = player_pawns & ~(_rook_pins | _bishop_pins);
                bitScan(current_pawns)
                {
                    int pawn_idx = bit_index(current_pawns);

                    U64 legal_pawn_moves;
                    if constexpr (WhiteMove)
                    {
                        legal_pawn_moves = get_white_pawn_attack(pawn_idx) & _occ_bitboards[enemySide<WhiteMove>()] & _checkmask;
                    }
                    else
                    {
                        legal_pawn_moves = get_black_pawn_attack(pawn_idx) & _occ_bitboards[enemySide<WhiteMove>()] & _checkmask;
                    }

                    legal_pawn_moves |= forward_pawn_moves & cols_get[pawn_idx % 8];
                    bitScan(legal_pawn_moves)
                    {
                        int legal_move_idx = bit_index(legal_pawn_moves);
                        pushPawnMoves<WhiteMove>(moves, pawn_idx, legal_move_idx, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx));
                    }
                }
            }

            if constexpr (ENPoss)
            {

                // constexpr checking if there are any pawns that are even on the proper rank to capture en passant
                // this will prune a lot of calculating in the begining of the game as there are a lot of not
                // capturable pawn double pushes.

                if(enPassantPossible<WhiteMove>(_piece_bitboards[playerPiece<WhiteMove>(PAWN)] & ~_rook_pins, (1UL << _en_passant) & _checkmask))
                {

                    U64 en_passant_source;
                    if constexpr(WhiteMove)
                    {
                        en_passant_source = _piece_bitboards[playerPiece<WhiteMove>(PAWN)] & get_black_pawn_attack(_en_passant);
                    }
                    else
                    {
                        en_passant_source = _piece_bitboards[playerPiece<WhiteMove>(PAWN)] & get_white_pawn_attack(_en_passant);
                    }

                    // there are maximum 2(2 is very very rare 1 is more likely soo this is just one move to analyze)
                    // second condition stops the loop if there is a check and en passant doens't stop it
                    
                    bitScan(en_passant_source)
                    {
                        int ep_idx = bit_index(en_passant_source);

                        // make move
                        CLEAR_BIT(_occ_bitboards[Side::BOTH], ep_idx); // clear pawn from previous
                        SET_BIT(_occ_bitboards[Side::BOTH], _en_passant); // set pawn on the en passant square
                        if constexpr(WhiteMove)
                        {
                            CLEAR_BIT(_occ_bitboards[Side::BOTH], _en_passant+8);
                        }
                        else
                        {
                            CLEAR_BIT(_occ_bitboards[Side::BOTH], _en_passant-8);
                        }

                        if(_valid_en_passant<WhiteMove>())
                        {
                            moves.push_back(createMove(ep_idx, _en_passant, playerPiece<WhiteMove>(PAWN), no_piece, true, false, true, false, false, true));
                        }

                        // unmake move
                        SET_BIT(_occ_bitboards[Side::BOTH], ep_idx);
                        if constexpr(WhiteMove)
                        {
                            SET_BIT(_occ_bitboards[Side::BOTH], _en_passant+8);
                        }
                        else
                        {
                            SET_BIT(_occ_bitboards[Side::BOTH], _en_passant-8);
                        }
                        CLEAR_BIT(_occ_bitboards[Side::BOTH], _en_passant);
                    }
                }
            }
        }

        /* ------------------------------- KING MOVES ------------------------------- */

        {
            U64 player_king = get_king_attack(king_idx) & ~_enemy_attack_bb & ~_occ_bitboards[playerSide<WhiteMove>()] & sadsquare_bb;

            bitScan(player_king)
            {
                int legal_move_idx = bit_index(player_king);
                moves.push_back(createMove(king_idx, legal_move_idx, playerPiece<WhiteMove>(KING), no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, true, false, false));
            }

            if constexpr ((WhiteMove && Kcastle) || (!WhiteMove && kcastle))
            {
                // player kingside castle possible
                if( !(kingsideCastleMask<WhiteMove>() & (_enemy_attack_bb | (_occ_bitboards[playerSide<WhiteMove>()] & ~_piece_bitboards[playerPiece<WhiteMove>(KING)]))) )
                {
                    moves.push_back(createMove(king_idx, kingsideCastleTarget<WhiteMove>(), playerPiece<WhiteMove>(KING), no_piece, false, false, false, true, false, false));
                }
            }
            if constexpr ((WhiteMove && Qcastle) || (!WhiteMove && qcastle))
            {
                // player queenside castle possible
                if( !(queensideCastleMask<WhiteMove>() & (_enemy_attack_bb | (_occ_bitboards[playerSide<WhiteMove>()] & ~_piece_bitboards[playerPiece<WhiteMove>(KING)]))) )
                {
                    moves.push_back(createMove(king_idx, queensideCastleTarget<WhiteMove>(), playerPiece<WhiteMove>(KING), no_piece, false, false, false, true, false, false));
                }
            }
        }
    }
    else
    {
        /* only king moves (no castling) */
        U64 sadsquare_bb = 0;
        U64 op_checkmask = _checkmask & get_king_attack(king_idx);
        bitScan(op_checkmask)
        {
            sadsquare_bb |= _get_sadsquare(king_idx, LS1B(op_checkmask));
        }
        sadsquare_bb = ~sadsquare_bb;

        U64 player_king = get_king_attack(king_idx) & ~_enemy_attack_bb & ~_occ_bitboards[playerSide<WhiteMove>()] & sadsquare_bb;

        bitScan(player_king)
        {
            int legal_move_idx = bit_index(player_king);
            moves.push_back(createMove(king_idx, legal_move_idx, playerPiece<WhiteMove>(KING), no_piece, GET_BIT(_occ_bitboards[enemySide<WhiteMove>()], legal_move_idx), false, false, true, false, false));
        }
    }
}



/* -------------------------------------------------------------------------- */
/*                       MOVE GENERATION TEMPLATE WRAPER                      */
/* -------------------------------------------------------------------------- */

void Board::getMoves()
{
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 15) { _getMoves<true, false, true, true, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 14) { _getMoves<true, false, true, true, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 13) { _getMoves<true, false, true, true, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 12) { _getMoves<true, false, true, true, false, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 11) { _getMoves<true, false, true, false, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 10) { _getMoves<true, false, true, false, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 9) { _getMoves<true, false, true, false, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 8) { _getMoves<true, false, true, false, false, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 7) { _getMoves<true, false, false, true, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 6) { _getMoves<true, false, false, true, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 5) { _getMoves<true, false, false, true, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 4) { _getMoves<true, false, false, true, false, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 3) { _getMoves<true, false, false, false, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 2) { _getMoves<true, false, false, false, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 1) { _getMoves<true, false, false, false, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant == NO_SQ && _castle_rights == 0) { _getMoves<true, false, false, false, false, false>();}

    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 15) { _getMoves<true, true, true, true, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 14) { _getMoves<true, true, true, true, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 13) { _getMoves<true, true, true, true, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 12) { _getMoves<true, true, true, true, false, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 11) { _getMoves<true, true, true, false, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 10) { _getMoves<true, true, true, false, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 9) { _getMoves<true, true, true, false, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 8) { _getMoves<true, true, true, false, false, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 7) { _getMoves<true, true, false, true, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 6) { _getMoves<true, true, false, true, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 5) { _getMoves<true, true, false, true, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 4) { _getMoves<true, true, false, true, false, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 3) { _getMoves<true, true, false, false, true, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 2) { _getMoves<true, true, false, false, true, false>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 1) { _getMoves<true, true, false, false, false, true>();}
    if(_side_to_move == Side::WHITE && _en_passant != NO_SQ && _castle_rights == 0) { _getMoves<true, true, false, false, false, false>();}

    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 15) { _getMoves<false, false, true, true, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 14) { _getMoves<false, false, true, true, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 13) { _getMoves<false, false, true, true, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 12) { _getMoves<false, false, true, true, false, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 11) { _getMoves<false, false, true, false, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 10) { _getMoves<false, false, true, false, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 9) { _getMoves<false, false, true, false, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 8) { _getMoves<false, false, true, false, false, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 7) { _getMoves<false, false, false, true, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 6) { _getMoves<false, false, false, true, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 5) { _getMoves<false, false, false, true, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 4) { _getMoves<false, false, false, true, false, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 3) { _getMoves<false, false, false, false, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 2) { _getMoves<false, false, false, false, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 1) { _getMoves<false, false, false, false, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant == NO_SQ && _castle_rights == 0) { _getMoves<false, false, false, false, false, false>();}

    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 15) { _getMoves<false, true, true, true, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 14) { _getMoves<false, true, true, true, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 13) { _getMoves<false, true, true, true, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 12) { _getMoves<false, true, true, true, false, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 11) { _getMoves<false, true, true, false, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 10) { _getMoves<false, true, true, false, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 9) { _getMoves<false, true, true, false, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 8) { _getMoves<false, true, true, false, false, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 7) { _getMoves<false, true, false, true, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 6) { _getMoves<false, true, false, true, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 5) { _getMoves<false, true, false, true, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 4) { _getMoves<false, true, false, true, false, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 3) { _getMoves<false, true, false, false, true, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 2) { _getMoves<false, true, false, false, true, false>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 1) { _getMoves<false, true, false, false, false, true>();}
    if(_side_to_move == Side::BLACK && _en_passant != NO_SQ && _castle_rights == 0) { _getMoves<false, true, false, false, false, false>();}
}
