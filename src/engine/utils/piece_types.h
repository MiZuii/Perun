#pragma once

#include <inttypes.h>

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

typedef uint8_t Piece_t;