#pragma once

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

int log2_64 (uint64_t value);
