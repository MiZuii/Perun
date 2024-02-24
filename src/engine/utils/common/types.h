#pragma once

#include <inttypes.h>
#include <string.h>

#define _ForceInline __always_inline
#define _Inline inline

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

typedef int16_t ScoreVal_t;

// also denotes ply (the engine works only on ply values not full moves)
typedef uint8_t Depth_t;
static constexpr Depth_t DEPTH_TYPE_MAX = UINT8_MAX;

typedef std::string FEN_t;

/* --------------------------- PIECE TYPES DEFINES -------------------------- */

enum Side {
    WHITE,
    BLACK,
    BOTH
};

constexpr Side opositeSide(Side side)
{
    return side == Side::WHITE ? Side::BLACK : Side::WHITE;
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
constexpr std::array<Piece, 6> whitePieces = {K, Q, B, N, R, P};
constexpr std::array<Piece, 6> blackPieces = {k, q, b, n, r, p};
constexpr Piece allPieces[12] = {K, Q, B, N, R, P, k, q, b, n, r, p};


constexpr std::array<Piece, 6> getColoredPieces(Side sd)
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

constexpr PieceType getPieceType(Piece piece)
{
    if( piece == no_piece )
    {
        return NONE;
    }

    return pieces[piece % 6];
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
    if (side == Side::WHITE)
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
bits 12-15  -> source piece
bits 16-19  -> promoted piece (no_piece enum if no promotion (12))
bit  20     -> capture flag
bit  21     -> double push flag
bit  22     -> en passant flag
bit  23     -> castle flag
bit  24     -> rook move (this flag is used to update castling rights (queens also have this flag!))
bit  25     -> normal pawn push flag

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

/* ----------------------------- SQUARE DEFINES ----------------------------- */

enum Field
{
    _H,
    _G,
    _F,
    _E,
    _D,
    _C,
    _B,
    _A
};

enum Rank
{
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8
};

enum Square
{
    H1, G1, F1, E1, D1, C1, B1, A1,
    H2, G2, F2, E2, D2, C2, B2, A2,
    H3, G3, F3, E3, D3, C3, B3, A3,
    H4, G4, F4, E4, D4, C4, B4, A4,
    H5, G5, F5, E5, D5, C5, B5, A5,
    H6, G6, F6, E6, D6, C6, B6, A6,
    H7, G7, F7, E7, D7, C7, B7, A7,
    H8, G8, F8, E8, D8, C8, B8, A8,

    NO_SQ = 64
};

/* --------------------------------- RANDOM --------------------------------- */
#define FEN_REGEX "^(([PRNBKQprnbkq1-8]){1,8}\\/){7}(([PRNBKQprnbkq1-8]){1,8}) (w|b) (-|(K?Q?k?)q|(K?Q?kq?)|(K?Qk?q?)|(KQ?k?q?)) (-|[a-h][1-8]) (0|[1-9][0-9]*) (0|[1-9][0-9]*)$"

/* -------------------------------------------------------------------------- */
/*                                   SEARCH                                   */
/* -------------------------------------------------------------------------- */

#define EVAL_INF 10000

#define SEARCH_INF 200

enum SearchLimitType
{
    DEPTH_LIM,
    TIME_LIM,
    NODE_LIM,

    NO_LIM
};

/* -------------------------------------------------------------------------- */
/*                                   HASHING                                  */
/* -------------------------------------------------------------------------- */

enum NodeType
{
    EXACT,
    ALPHA,
    BETA
};

static constexpr Depth_t UNKNOWN_VAL = DEPTH_TYPE_MAX;
