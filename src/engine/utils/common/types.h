#pragma once

#include <inttypes.h>
#include <string.h>

#define _ForceInline __always_inline
#define _Inline inline

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

typedef int32_t ScoreVal_t;

/* --------------------------- PIECE TYPES DEFINES -------------------------- */

enum Side {
    WHITE,
    BLACK,
    BOTH
};

constexpr Side opositeSide(Side side)
{
    if (side == Side::WHITE)
    {
        return Side::BLACK;
    }
    else 
    {
        return Side::WHITE;
    }
}

enum PieceType {
    KING,
    QUEEN,
    BISHOP,
    KNIGHT,
    ROOK,
    PAWN,
    NONE
};

/* If enumeration order is to be changed the opositePiece method needs to be updated */
enum Piece {
    K,
    Q,
    B,
    N,
    R,
    P,
    k,
    q,
    b,
    n,
    r,
    p,
    no_piece
};

constexpr PieceType pieces[6] = {KING, QUEEN, BISHOP, KNIGHT, ROOK ,PAWN};
constexpr Piece whitePieces[6] = {K, Q, B, N, R, P};
constexpr Piece blackPieces[6] = {k, q, b, n, r, p};


constexpr const Piece *getColoredPieces(Side sd)
{
    if (Side::WHITE == sd)
    {
        return whitePieces;
    }
    else 
    {
        return blackPieces;
    }
}

constexpr Piece convertPiece(PieceType pt, Side sd)
{
    if(Side::WHITE == sd)
    {
        return static_cast<Piece>(pt);
    }
    else
    {
        return static_cast<Piece>(pt+6);
    }
}

constexpr Piece getColoredQueen(Side side)
{
    if(side == Side::WHITE)
    {
        return Q;
    }
    else
    {
        return q;
    }
}

constexpr bool isWhitePiece(Piece piece)
{
    if (piece >= Piece::K && piece <= Piece::P)
    {
        return true;
    }
    return false;
}

constexpr bool isBlackPiece(Piece piece)
{
    if (piece >= Piece::k && piece <= Piece::p)
    {
        return true;
    }
    return false;
}

constexpr Piece opositePiece(Piece piece) {
    if (isWhitePiece(piece)) {
        return static_cast<Piece>(static_cast<int>(piece) + 6);
    }

    else if (isBlackPiece(piece)) {
        return static_cast<Piece>(static_cast<int>(piece) - 6);
    }

    return Piece::no_piece;
}

/* -------------------------- MOVE DEFINES AND TYPE ------------------------- */

typedef U32 Move_t;
/* Move_t type description bit by bit
bits 0-6    -> source square of the piece
bits 6-11   -> target square of the piece to move to
bits 12-16  -> promoted piece (no_piece enum if no promotion (12))
bit  16     -> capture flag
bit  17     -> double push flag
bit  18     -> en passant flag
bit  19     -> castle flag
bit  20     -> rook move (this flag is used to update castling rights (queens also have this flag!))

Below are flag getters
*/

#define SRC_MASK 0b111111
#define TRG_MASK 0b111111000000
#define TRG_SHIFT 6
#define SRP_MASK 0b1111000000000000
#define SRP_SHIFT 12
#define PRP_MASK 0b11110000000000000000
#define PRP_SHIFT 16

#define CAP_FLAG_MASK 0b100000000000000000000
#define CAP_FLAG_SHIFT 20
#define DPP_FLAG_MASK 0b1000000000000000000000
#define DPP_FLAG_SHIFT 21
#define ENP_FLAG_MASK 0b10000000000000000000000
#define ENP_FLAG_SHIFT 22
#define CST_FLAG_MASK 0b100000000000000000000000
#define CST_FLAG_SHIFT 23
#define ROK_FLAG_MASK 0b1000000000000000000000000
#define ROK_FLAG_SHIFT 24
#define NPP_FLAG_MASK 0b10000000000000000000000000
#define NPP_FLAG_SHIFT 25

_ForceInline int getSourceSquare(Move_t move)
{
    return move & SRC_MASK;
}

_ForceInline int getTargetSquare(Move_t move)
{
    return (move & TRG_MASK) >> TRG_SHIFT;
}

_ForceInline Piece getSourcePiece(Move_t move)
{
    return static_cast<Piece>((move & SRP_MASK) >> SRP_SHIFT);
}

_ForceInline Piece getPromotionPiece(Move_t move)
{
    return static_cast<Piece>((move & PRP_MASK) >> PRP_SHIFT);
}

_ForceInline bool getCaptureFlag(Move_t move)
{
    return move & CAP_FLAG_MASK;
}

_ForceInline bool getDoublePawnPushFlag(Move_t move)
{
    return move & DPP_FLAG_MASK;
}

_ForceInline bool getEnPassantFlag(Move_t move)
{
    return move & ENP_FLAG_MASK;
}

_ForceInline bool getCastleFlag(Move_t move)
{
    return move & CST_FLAG_MASK;
}

_ForceInline bool getRookFlag(Move_t move)
{
    return move & ROK_FLAG_MASK;
}

_ForceInline bool getPawnPushFlag(Move_t move)
{
    return move & NPP_FLAG_MASK;
}

_ForceInline Move_t createMove(int source_square,
                             int target_square,
                             Piece source_piece,
                             Piece promoted_piece,
                             bool capture, 
                             bool double_pawn_push, 
                             bool en_passant, 
                             bool castle,
                             bool rook_move,
                             bool pawn_push)
{
    return ((Move_t)castle << CST_FLAG_SHIFT) | ((Move_t)en_passant << ENP_FLAG_SHIFT) | 
           ((Move_t)capture << CAP_FLAG_SHIFT) | ((Move_t)double_pawn_push << DPP_FLAG_SHIFT) |
           ((Move_t)rook_move << ROK_FLAG_SHIFT) | ((Move_t)source_piece << SRP_SHIFT ) |
           ((Move_t)pawn_push << NPP_FLAG_SHIFT) |
           (promoted_piece << PRP_SHIFT) | (target_square << TRG_SHIFT) | (source_square);
}



typedef std::string FEN_t;

/* ----------------------------- SQUARE DEFINES ----------------------------- */

#define _A 7
#define _B 6
#define _C 5
#define _D 4
#define _E 3
#define _F 2
#define _G 1
#define _H 0

#define _1 0
#define _2 1
#define _3 2
#define _4 3
#define _5 4
#define _6 5
#define _7 6
#define _8 7

#define A1 7
#define B1 6
#define C1 5
#define D1 4
#define E1 3
#define F1 2
#define G1 1
#define H1 0

#define A2 15
#define B2 14
#define C2 13
#define D2 12
#define E2 11
#define F2 10
#define G2 9
#define H2 8

#define A3 23
#define B3 22
#define C3 21
#define D3 20
#define E3 19
#define F3 18
#define G3 17
#define H3 16

#define A4 31
#define B4 30
#define C4 29
#define D4 28
#define E4 27
#define F4 26
#define G4 25
#define H4 24

#define A5 39
#define B5 38
#define C5 37
#define D5 36
#define E5 35
#define F5 34
#define G5 33
#define H5 32

#define A6 47
#define B6 46
#define C6 45
#define D6 44
#define E6 43
#define F6 42
#define G6 41
#define H6 40

#define A7 55
#define B7 54
#define C7 53
#define D7 52
#define E7 51
#define F7 50
#define G7 49
#define H7 48

#define A8 63
#define B8 62
#define C8 61
#define D8 60
#define E8 59
#define F8 58
#define G8 57
#define H8 56

#define NO_SQ 64

/* --------------------------------- RANDOM --------------------------------- */
#define FEN_REGEX "^(([PRNBKQprnbkq1-8]){1,8}\\/){7}(([PRNBKQprnbkq1-8]){1,8}) (w|b) (-|(K?Q?k?)q|(K?Q?kq?)|(K?Qk?q?)|(KQ?k?q?)) (-|[a-h][1-8]) (0|[1-9][0-9]*) (0|[1-9][0-9]*)$"

