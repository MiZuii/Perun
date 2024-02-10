#pragma once

#include "types.h"
#include "includes.h"

#define GET_BIT(board, x) ((board) & (1UL << (x)))
#define CLEAR_BIT(board, x) board = ((board) & ~(1UL << (x)))
#define TOGGLE_BIT(board, x) board = ((board) ^ (1UL << (x)))
#define SET_BIT(board, x) board = (((board) & ~(1UL << (x))) ^ (1UL << (x)))
#define LS1B(x) ((x) & (-(x)))

enum Columns
{
    A_COL = 0x8080808080808080,
    B_COL = 0x4040404040404040,
    C_COL = 0x2020202020202020,
    D_COL = 0x1010101010101010,
    E_COL = 0x0808080808080808,
    F_COL = 0x0404040404040404,
    G_COL = 0x0202020202020202,
    H_COL = 0x0101010101010101
};

enum Rows
{
    ROW_8 = 0xFF00000000000000,
    ROW_7 = 0x00FF000000000000,
    ROW_6 = 0x0000FF0000000000,
    ROW_5 = 0x000000FF00000000,
    ROW_4 = 0x00000000FF000000,
    ROW_3 = 0x0000000000FF0000,
    ROW_2 = 0x000000000000FF00,
    ROW_1 = 0x00000000000000FF
};

#define IS_A_COL(board) ((board) & A_COL)
#define IS_AB_COL(board) (((board) & A_COL) | ((board) & B_COL))
#define IS_H_COL(board) ((board) & H_COL)
#define IS_GH_COL(board) (((board) & H_COL) | ((board) & G_COL))

#define IS_ROW_1(board) ((board) & ROW_1)
#define IS_ROW_12(board) (((board) & ROW_1) | ((board) & ROW_2))
#define IS_ROW_8(board) ((board) & ROW_8)
#define IS_ROW_78(board) (((board) & ROW_8) | ((board) & ROW_7))

#ifdef __BMI__
#define bitScan(x) for(;x;x = _blsr_u64(x))
#else
#define bitScan(x) for(;x;x = x ^ LS1B(x))
#endif

#ifdef __BMI__
#else
const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

int bitScanReverse(U64 bb) {
   const U64 debruijn64 = 0x03f79d71b4cb0a89UL;
   assert (bb != 0);
   bb |= bb >> 1; 
   bb |= bb >> 2;
   bb |= bb >> 4;
   bb |= bb >> 8;
   bb |= bb >> 16;
   bb |= bb >> 32;
   return index64[(bb * debruijn64) >> 58];
}
#endif

_ForceInline int bit_index(uint64_t x)
{
#ifdef __BMI__
    return _tzcnt_u64(x);
#else
    return bitScanReverse(x);
#endif
}

_ForceInline int bit_count(uint64_t bitboard)
{
#ifdef __POPCNT__
    return _popcnt64(bitboard);
#else
    int count = 0;
    while (x) {
        count++;
        x &= x - 1; // reset LS1B
    }
    return count;
#endif
}

constexpr uint64_t cols_get[8] = {H_COL, G_COL, F_COL, E_COL, D_COL, C_COL, B_COL, A_COL};
constexpr uint64_t rows_get[8] = {ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8};
