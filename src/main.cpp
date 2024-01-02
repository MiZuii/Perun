#include "engine/utils/common/includes.h"
#include "engine/utils/common/types.h"
#include "engine/utils/common/bit_opers.h"

#include "engine/utils/attack_tables_gen.h"
#include "engine/utils/attack_tables.h"
#include "engine/board/board_repr.h"

int main(void) {
#if DEBUG

    generate_bishop_attack(bishop_attack, bishop_relevant_bits, bishop_mask, bishop_magic);

    for( int i=0; i<64; i++) {
        // std::cout << king_attack[i] << std::endl;
        // PureBitBoard b(knight_attack[i]);
        // std::cout << b << std::endl;
    }
#endif
    return 0;
}