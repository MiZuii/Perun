#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

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

class Board
{
private:
    template<bool WhiteMove, bool ENPoss, bool Kcastle, bool Qcastle, bool kcastle, bool qcastle>
    std::vector<U16> _getMoves();

public:   
    U64 _piece_bitboards[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    U64 _occ_bitboards[3] = {0, 0, 0};

    /*Castle rights explanation
    1000 - white King side castle
    0100 - white Queen side castle
    0010 - black King side castle
    0001 - black Queen side castle
    */

    Side _side_to_move=Side::white;
    U8 _castle_rights=0, _en_passant=NO_SQ;
    int _halfmove_clock=0, _fullmove_clock=0;

    // methods

    Board();    // FEN_t of starting position is used to init
    Board(FEN_t fen);
    Board(const Board &board) = default;
    Board(Board &&board) = default;
    ~Board() = default;

    /* move format description -> the int is divided into 3 parts flags/src_index/target_index 
    both indexes describe int in range <0, 64> soo they need 6 bits. Because of that the int
    should be an 16bit unsigned integer. We are left with 4 bits.
    first 2 bits we reserve for loosing castle rights.
    0b0001... - bit responsible for queeside castle rights loos (if lost than 1)
    0b0010... - bit responsible for kingside castle rights loos (if lost than 1)
    This needs to be done because we can loose both rights with one move.
    This leaves us with 4 custom flags
    silent|capture|pawn_double_push|(space for one flag)
    */
    std::vector<U16> getMoves(); // template wrapper

    _ForceInline U64 get_white_pawn_attack(int idx);
    _ForceInline U64 get_black_pawn_attack(int idx);
    _ForceInline U64 get_knight_attack(int idx);
    _ForceInline U64 get_king_attack(int idx);
    _ForceInline U64 get_bishop_attack(int idx);
    _ForceInline U64 get_rook_attack(int idx);
    _ForceInline U64 get_queen_attack(int idx);
    _ForceInline U64 get_bishop_checkmask(int idx, int king_idx);
    _ForceInline U64 get_rook_checkmask(int idx, int king_idx);

    static Piece charToPiece(char pieceChar);
    static char PieceToChar(Piece piece);
    static wchar_t PieceToWChar(Piece piece);
    static U8 squareToInt(char field, char rank);
    static char intToField(U8 square);
    static char intToRank(U8 square);
    static std::string moveToString(U16 move);

    static bool validFEN(FEN_t fen);

    std::string toString() const;
    std::wstring toWString() const;
    FEN_t getFEN();
};