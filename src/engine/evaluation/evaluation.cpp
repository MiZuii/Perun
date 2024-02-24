#include "evaluation.h"

_ForceInline int materialDiff(U64 (&piece_bb)[12], PieceType piece_to_calc)
{
    return bit_count(piece_bb[playerPiece<true>(piece_to_calc)]) -
        bit_count(piece_bb[playerPiece<false>(piece_to_calc)]);
}

template<bool WhiteMove>
int evaluate(Board &board)
{
    if(board.moves.empty())
    {
        return board.getCheckers() ? ( WhiteMove ? -EVAL_INF : EVAL_INF) : 0;
    }

    // non relative
    const ScoreVal_t material_score = 
        materialDiff(board._piece_bitboards, QUEEN)*QUEEN_SCORE +
        materialDiff(board._piece_bitboards, ROOK)*ROOK_SCORE +
        materialDiff(board._piece_bitboards, BISHOP)*BISHOP_SCORE +
        materialDiff(board._piece_bitboards, KNIGHT)*KNIGHT_SCORE +
        materialDiff(board._piece_bitboards, PAWN)*PAWN_SCORE;

    /* ------------------------- FILL PLAYER ATTACK MAP ------------------------- */

    U64 player_attack_bb = 0;

    {
        const U64 player_queen = board._piece_bitboards[playerPiece<WhiteMove>(QUEEN)];


        // player bishop attack processing
        {

            U64 player_bishops = board._piece_bitboards[enemyPiece<WhiteMove>(BISHOP)] | player_queen;
            bitScan(player_bishops)
            {
                int bishop_index = bit_index(player_bishops);
                U64 bishop_attack = board.get_bishop_attack(bishop_index);
                player_attack_bb |= bishop_attack;
            }
        }

        //player rook attack processing
        {
            U64 enemy_rooks = board._piece_bitboards[enemyPiece<WhiteMove>(ROOK)] | player_queen;
            bitScan(enemy_rooks)
            {
                int rook_index = bit_index(enemy_rooks);
                U64 rook_attack = board.get_rook_attack(rook_index);
                player_attack_bb |= rook_attack;
            }
        }

        // player knight attack processing
        {
            U64 player_knights = board._piece_bitboards[enemyPiece<WhiteMove>(KNIGHT)];
            bitScan(player_knights)
            {
                int knight_index = bit_index(player_knights);
                U64 knight_attack = board.get_knight_attack(knight_index);
                player_attack_bb |= knight_attack;
            }
        }

        // player king attack processing
        {
            U64 player_king = board._piece_bitboards[enemyPiece<WhiteMove>(KING)];
            bitScan(player_king)
            {
                player_attack_bb |= board.get_king_attack(bit_index(player_king));
            }
        }

        // player pawn processing
        {
            U64 player_pawns = board._piece_bitboards[enemyPiece<WhiteMove>(PAWN)];
            bitScan(player_pawns)
            {
                if constexpr (WhiteMove)
                {
                    int pawn_index = bit_index(player_pawns);
                    U64 current_pawn_attack = board.get_white_pawn_attack(pawn_index);
                    player_attack_bb |= current_pawn_attack;
                }
                else
                {
                    int pawn_index = bit_index(player_pawns);
                    U64 current_pawn_attack = board.get_black_pawn_attack(pawn_index);
                    player_attack_bb |= current_pawn_attack;
                }
            }
        }
    }

    ScoreVal_t position_activity = 8*(bit_count(player_attack_bb) - bit_count(board._enemy_attack_bb));
    if constexpr (!WhiteMove)
    {
        position_activity = -position_activity;
    }

    return material_score + position_activity;
}

template int evaluate<true>(Board &board);
template int evaluate<false>(Board &board);
