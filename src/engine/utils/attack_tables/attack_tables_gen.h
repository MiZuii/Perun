#pragma once

#include "../common/types.h"

#if DEBUG

#define SIZE 64
#define BISHOP_ATTACK_SIZE 512
#define ROOK_ATTACK_SIZE 4096

void generate_white_pawn_attack(U64 (&arr)[SIZE]);
void generate_black_pawn_attack(U64 (&arr)[SIZE]);
void generate_knight_attack(U64 (&arr)[SIZE]);
void generate_king_attack(U64 (&arr)[SIZE]);

void generate_bishop_attack(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE]);
void generate_rook_attack(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE]);

void generate_bishop_pin_attack(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE]);
void generate_rook_pin_attack(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE]);

void generate_rook_checkmask(U64 (&attack)[SIZE][SIZE]);
void generate_bishop_checkmask(U64 (&attack)[SIZE][SIZE]);

void generate_sadsquares(U64 (&arr)[SIZE][SIZE+1]);

U64 make_bishop_mask(int sq);
U64 make_rook_mask(int sq);
U8 count_bits(U64 pos);
U64 bishop_attack_otf(U64 occ, int sq);
U64 rook_attack_otf(U64 occ, int sq);

#endif