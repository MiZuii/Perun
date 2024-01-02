#pragma once

#include "common/includes.h"
#include "common/types.h"

#define SIZE 64
#define BISHOP_ATTACK_SIZE 512
#define ROOK_ATTACK_SIZE 4096

/* -------------------------------------------------------------------------- */
/*                            SIMPLE ATTACK TABLES                            */
/* -------------------------------------------------------------------------- */

extern U64 white_pawn_attack[SIZE];
extern U64 black_pawn_attack[SIZE];
extern U64 knight_attack[SIZE];
extern U64 king_attack[SIZE];
extern U64 bishop_attack[SIZE][BISHOP_ATTACK_SIZE];
extern U64 rook_attack[SIZE][ROOK_ATTACK_SIZE];

/* -------------------------------------------------------------------------- */
/*                             MAGIC ATTACK TABLES                            */
/* -------------------------------------------------------------------------- */

//relevant bits
extern U8 bishop_relevant_bits[SIZE];
extern U8 rook_relevant_bits[SIZE];

//masks
extern U64 bishop_mask[SIZE];
extern U64 rook_mask[SIZE];

//magic
extern U64 bishop_magic[SIZE];
extern U64 rook_magic[SIZE];

// getter functins
U64 get_bishop_attack(U64 occ, int sq);
U64 get_rook_attack(U64 occ, int sq);
