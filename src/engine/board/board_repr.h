#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "../utils/attack_tables/attack_tables.h"

#define STARTING_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/* -------------------------------------------------------------------------- */
/*                                  BITBOARDS                                 */
/* -------------------------------------------------------------------------- */

/* class providing a wraper with usefull debug functions 
for bitboard, the bitboards themselves are just U64 numbers.
The bitboard class would be not efficient. For simplifying
the debuging a bit board wraper class provides methodes
applied to bitboards*/
#if DEBUG

class BitBoardWrap
{
public:

    U64 _bit_board;

    BitBoardWrap();
    BitBoardWrap(U64 board);
    ~BitBoardWrap() = default;

    static std::string bitboard_repr(U64 bb);
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const BitBoardWrap& obj);
};

#endif

/* -------------------------------------------------------------------------- */
/*                                    BOARD                                   */
/* -------------------------------------------------------------------------- */

template<bool WhiteMove>
_ForceInline constexpr Side playerSide()
{
    if constexpr(WhiteMove)
    {
        return Side::WHITE;
    }
    else
    {
        return Side::BLACK;
    }
}

template<bool WhiteMove>
_ForceInline constexpr Side enemySide()
{
    if constexpr(WhiteMove)
    {
        return Side::BLACK;
    }
    else
    {
        return Side::WHITE;
    }
}

template<bool WhiteMove>
_ForceInline constexpr Piece playerPiece(PieceType piece_type) {
    return convertPiece(piece_type, playerSide<WhiteMove>());
}

template<bool WhiteMove>
_ForceInline constexpr Piece enemyPiece(PieceType piece_type) {
    return convertPiece(piece_type, enemySide<WhiteMove>());
}
class Board
{
private:

    U64 _piece_bitboards[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    U64 _occ_bitboards[3] = {0, 0, 0};

    Side _side_to_move=Side::WHITE;
    int _castle_rights=0, _en_passant=NO_SQ;
    int _halfmove_clock=0, _fullmove_clock=0;

    // refreshable variables
    U64 _enemy_attack_bb=0, _checkers=0, _rook_pins=0, _bishop_pins=0, _checkmask=0;
    int king_idx;

    template<bool WhiteMove, bool ENPoss, bool Kcastle, bool Qcastle, bool kcastle, bool qcastle>
    void _getMoves();

    /* -------------------------------------------------------------------------- */
    /*                       MOVE GENERATION HELP FUNCTIONS                       */
    /* -------------------------------------------------------------------------- */

    template<bool WhiteMove>
    void _refresh();
    template<bool WhiteMove>
    _ForceInline void _register_rook_pins();
    template<bool WhiteMove>
    _ForceInline void _register_bishop_pins();
    template<bool WhiteMove>
    _ForceInline bool _valid_en_passant();

    _ForceInline U64 get_white_pawn_attack(int idx) {return white_pawn_attack[idx];}
    _ForceInline U64 get_black_pawn_attack(int idx) {return black_pawn_attack[idx];}
    _ForceInline U64 get_knight_attack(int idx) {return knight_attack[idx];}
    _ForceInline U64 get_king_attack(int idx) {return king_attack[idx];}
    _ForceInline U64 get_bishop_attack(int idx) {
        return _get_bishop_attack(_occ_bitboards[BOTH], idx);
    }
    _ForceInline U64 get_rook_attack(int idx) {
        return _get_rook_attack(_occ_bitboards[BOTH], idx);
    }
    _ForceInline U64 get_bishop_attack_pin(int idx) {
        return _get_bishop_attack_pin(_occ_bitboards[BOTH], idx);
    }
    _ForceInline U64 get_rook_attack_pin(int idx) {
        return _get_rook_attack_pin(_occ_bitboards[BOTH], idx);
    }
    _ForceInline U64 get_queen_attack(int idx) {
        return _get_bishop_attack(_occ_bitboards[BOTH], idx) |
           _get_rook_attack(_occ_bitboards[BOTH], idx);
    }
    _ForceInline U64 get_queen_attack_pin(int idx) {
        return _get_bishop_attack_pin(_occ_bitboards[BOTH], idx) |
           _get_rook_attack_pin(_occ_bitboards[BOTH], idx);
    }
    _ForceInline U64 get_bishop_checkmask(int idx, int king_idx) {return bishop_checkmask[king_idx][idx];}
    _ForceInline U64 get_rook_checkmask(int idx, int king_idx) {return rook_checkmask[king_idx][idx];}

    /* -------------------------------------------------------------------------- */
    /*                         MOVE MAKING HELP FUNCTIONS                         */
    /* -------------------------------------------------------------------------- */

    _Inline void movePiece(Side playing_side, int source_square, int target_square, Piece source_piece);

public:

    std::vector<Move_t> moves;
    template<bool WhiteMove>
    friend int evaluate(Board &board);
    _ForceInline bool sideToMove() {return _side_to_move;};

    Board();    // FEN_t of starting position is used to init
    Board(const FEN_t fen);
    Board(const Board &board, Move_t move);
    Board(const Board &board);
    Board(Board &&board);
    ~Board() = default;

    Board &operator=(const Board &);

    void getMoves(); // public template wrapper
    void clearMoves();
    Board& makeMove(Move_t move);
    Move_t createAmbiguousMove(Move_t move);

    static bool validFEN(FEN_t fen);

    std::string toString() const;
    std::wstring toWString() const;
    
    FEN_t getFEN();
    static Piece charToPiece(char pieceChar);
    static char PieceToChar(Piece piece);
    static wchar_t PieceToWChar(Piece piece);
    static U8 squareToInt(char field, char rank); // used outside
    static char intToField(U8 square);
    static char intToRank(U8 square);
    static std::string moveToString(Move_t move);
    static std::string moveToStringShort(Move_t move);
};
