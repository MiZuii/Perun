#pragma once

#include <inttypes.h>

typedef uint64_t U64;
typedef uint8_t U8;
typedef uint8_t Piece_t;

/* --------------------------- PIECE TYPES DEFINES -------------------------- */

#define PAWN_W      0b10000000
#define KNIGHT_W    0b01000000
#define BISHOP_W    0b11000000
#define ROOK_W      0b00100000
#define QUEEN_W     0b10100000
#define KING_W      0b01100000

#define PAWN_B      0b00000001
#define KNIGHT_B    0b00000010
#define BISHOP_B    0b00000011
#define ROOK_B      0b00000100
#define QUEEN_B     0b00000101
#define KING_B      0b00000110

#define IS_WHITE(piece) piece > 0b00010000
#define IS_BLACK(piece) piece < 0b00001000

/* ----------------------------- SQUARE DEFINES ----------------------------- */

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