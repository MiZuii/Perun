#pragma once

#include "../common/includes.h"
#include "../common/types.h"

#include "attack_tables_data.h"
#include "attack_tables_data_bishop.h"
#include "attack_tables_data_bishop_pin.h"
#include "attack_tables_data_rook.h"
#include "attack_tables_data_rook_pin.h"


// tmp
#include "../../board/board_repr.h"

#define SIZE 64
#define BISHOP_ATTACK_SIZE 512
#define ROOK_ATTACK_SIZE 4096

// getter functins
U64 _ForceInline _get_bishop_attack(U64 occ, int sq) {
   occ &= bishop_mask[sq];
   occ *= bishop_magic[sq];
   occ >>= 64-bishop_relevant_bits[sq];
   return bishop_attack[sq][occ];
}

U64 _ForceInline _get_rook_attack(U64 occ, int sq) {
   occ &= rook_mask[sq];
   occ *= rook_magic[sq];
   occ >>= 64-rook_relevant_bits[sq];
   return rook_attack[sq][occ];
}

U64 _ForceInline _get_bishop_attack_pin(U64 occ, int sq) {
   occ &= bishop_mask[sq];
   occ *= bishop_magic_pin[sq];
   occ >>= 64-bishop_relevant_bits[sq];
   return bishop_attack_pin[sq][occ];
}

U64 _ForceInline _get_rook_attack_pin(U64 occ, int sq) {
   occ &= rook_mask[sq];
   occ *= rook_magic_pin[sq];
   occ >>= 64-rook_relevant_bits[sq];
   return rook_attack_pin[sq][occ];
}

U64 _ForceInline _get_sadsquare(int kingsq, U64 checkmask)
{
    return sadsquare[kingsq][bit_index(checkmask & king_attack[kingsq])];
}
