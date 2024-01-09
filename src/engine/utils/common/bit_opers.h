#pragma once

#include "types.h"
#include "includes.h"

#define GET_BIT(board, x) ((board) & (1UL << (x)))
#define CLEAR_BIT(board, x) board = ((board) & ~(1UL << (x)))
#define TOGGLE_BIT(board, x) board = ((board) ^ (1UL << (x)))
#define SET_BIT(board, x) board = (((board) & ~(1UL << (x))) ^ (1UL << (x)))

#define A_COL 0x8080808080808080
#define B_COL 0x4040404040404040
#define C_COL 0x2020202020202020
#define D_COL 0x1010101010101010
#define E_COL 0x0808080808080808
#define F_COL 0x0404040404040404
#define G_COL 0x0202020202020202
#define H_COL 0x0101010101010101

#define ROW_8 0xFF00000000000000
#define ROW_7 0x00FF000000000000
#define ROW_6 0x0000FF0000000000
#define ROW_5 0x000000FF00000000
#define ROW_4 0x00000000FF000000
#define ROW_3 0x0000000000FF0000
#define ROW_2 0x000000000000FF00
#define ROW_1 0x00000000000000FF

#define IS_A_COL(board) ((board) & A_COL)
#define IS_AB_COL(board) (((board) & A_COL) | ((board) & B_COL))
#define IS_H_COL(board) ((board) & H_COL)
#define IS_GH_COL(board) (((board) & H_COL) | ((board) & G_COL))

#define IS_ROW_1(board) ((board) & ROW_1)
#define IS_ROW_12(board) (((board) & ROW_1) | ((board) & ROW_2))
#define IS_ROW_8(board) ((board) & ROW_8)
#define IS_ROW_78(board) (((board) & ROW_8) | ((board) & ROW_7))

#define LS1B(x) (x) & -(x)

constexpr uint64_t cols_get[8] = {H_COL, G_COL, F_COL, E_COL, D_COL, C_COL, B_COL, A_COL};
constexpr uint64_t rows_get[8] = {ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8};

constexpr int tab64[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5};

_ForceInline int log2_64 (uint64_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}

_ForceInline int bit_count(uint64_t bitboard)
{
    int counter=0;
    while(bitboard)
    {
        counter++;
        bitboard &= bitboard - 1;
    }
    return counter;
}

_ForceInline uint64_t leadingZeros(uint64_t x) {
    return __builtin_ctzll(LS1B(x));
}

// TODO: provide ifdefs for different compilers(__builtin_ctzll)
// function to be used in code
_ForceInline int bit_index(uint64_t x)
{
    // the bit index should return 64 if x == 0
    // the king sadsquares lookup relies on that fact
    return leadingZeros(x);
}

constexpr int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

constexpr U64 debruijn64 = 0x03f79d71b4cb0a89;

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
_ForceInline int bitScanForward(U64 bb) {
   assert(bb != 0);
   return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
}

_ForceInline U8 to_1_shift(uint64_t x) {
    return x >> __builtin_ctzll(x);
}
