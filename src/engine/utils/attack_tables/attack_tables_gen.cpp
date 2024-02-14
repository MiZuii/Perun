#include "../common/includes.h"
#include "../common/types.h"
#include "../common/bit_opers.h"

#include "attack_tables_gen.h"

#if DEBUG

/* ------------------------------- SIMPLE GEN ------------------------------- */

void generate_white_pawn_attack(U64 (&arr)[SIZE])
{
    U64 fill, base_pos;
    for (U64 i = 0; i < SIZE; i++)
    {
        base_pos = 1UL << i;

        if (IS_H_COL(base_pos))
        {
            fill = (base_pos << 9);
        }
        else if (IS_A_COL(base_pos))
        {
            fill = (base_pos << 7);
        }
        else
        {
            fill = (base_pos << 9) | (base_pos << 7);
        }

        arr[i] = fill;
    }
}

void generate_black_pawn_attack(U64 (&arr)[SIZE])
{
    U64 fill, base_pos;
    for (U64 i = 0; i < SIZE; i++)
    {
        base_pos = 1UL << i;

        if (IS_H_COL(base_pos))
        {
            fill = (base_pos >> 7);
        }
        else if (IS_A_COL(base_pos))
        {
            fill = (base_pos >> 9);
        }
        else
        {
            fill = (base_pos >> 9) | (base_pos >> 7);
        }

        arr[i] = fill;
    }
}

void generate_knight_attack(U64 (&arr)[SIZE])
{
    U64 fill, base_pos;
    for (int i = 0; i < SIZE; i++)
    {
        base_pos = 1UL << i;

        fill = 0;
        if (!IS_ROW_12(base_pos))
        {
            if (!IS_A_COL(base_pos))
            {
                fill |= base_pos >> 15;
            }
            if (!IS_H_COL(base_pos))
            {
                fill |= base_pos >> 17;
            }
        }
        if (!IS_ROW_78(base_pos))
        {
            if (!IS_A_COL(base_pos))
            {
                fill |= base_pos << 17;
            }
            if (!IS_H_COL(base_pos))
            {
                fill |= base_pos << 15;
            }
        }
        if (!IS_AB_COL(base_pos))
        {
            if (!IS_ROW_8(base_pos))
            {
                fill |= base_pos << 10;
            }
            if (!IS_ROW_1(base_pos))
            {
                fill |= base_pos >> 6;
            }
        }
        if (!IS_GH_COL(base_pos))
        {
            if (!IS_ROW_8(base_pos))
            {
                fill |= base_pos << 6;
            }
            if (!IS_ROW_1(base_pos))
            {
                fill |= base_pos >> 10;
            }
        }

        arr[i] = fill;
    }
}

void generate_king_attack(U64 (&arr)[SIZE])
{
    U64 fill, base_pos;
    for (U64 i = 0; i < SIZE; i++)
    {
        base_pos = 1UL << i;

        fill = 0;
        if (!(IS_A_COL(base_pos)))
        {
            fill |= base_pos << 1;
        }
        if (!(IS_H_COL(base_pos)))
        {
            fill |= base_pos >> 1;
        }
        if (!(IS_ROW_1(base_pos)))
        {
            fill |= base_pos >> 8;
        }
        if (!(IS_ROW_8(base_pos)))
        {
            fill |= base_pos << 8;
        }
        if (!(IS_A_COL(base_pos)) && !(IS_ROW_1(base_pos)))
        {
            fill |= base_pos >> 7;
        }
        if (!(IS_A_COL(base_pos)) && !(IS_ROW_8(base_pos)))
        {
            fill |= base_pos << 9;
        }
        if (!(IS_H_COL(base_pos)) && !(IS_ROW_1(base_pos)))
        {
            fill |= base_pos >> 9;
        }
        if (!(IS_H_COL(base_pos)) && !(IS_ROW_8(base_pos)))
        {
            fill |= base_pos << 7;
        }

        arr[i] = fill;
    }
}

/* -------------------------------- MAGIC GEN ------------------------------- */

U64 make_bishop_mask(int sq)
{
    U64 mask = 0;
    int rank = sq / 8, file = sq % 8, r, f;

    for (r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++)
    {
        SET_BIT(mask, r * 8 + f);
    }

    for (r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--)
    {
        SET_BIT(mask, r * 8 + f);
    }

    for (r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++)
    {
        SET_BIT(mask, r * 8 + f);
    }

    for (r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--)
    {
        SET_BIT(mask, r * 8 + f);
    }

    return mask;
}

U64 make_rook_mask(int sq)
{
    U64 mask = 0;
    int rank = sq / 8, file = sq % 8, r, f;

    for (r = rank + 1, f = file; r < 7; r++)
    {
        SET_BIT(mask, r * 8 + f);
    }

    for (r = rank, f = file + 1; f < 7; f++)
    {
        SET_BIT(mask, r * 8 + f);
    }

    for (r = rank - 1, f = file; r > 0; r--)
    {
        SET_BIT(mask, r * 8 + f);
    }

    for (r = rank, f = file - 1; f > 0; f--)
    {
        SET_BIT(mask, r * 8 + f);
    }

    return mask;
}

U8 count_bits(U64 pos)
{
    U8 counter = 0;
    while (pos)
    {
        counter += pos % 2;
        pos >>= 1;
    }

    return counter;
}

U64 bishop_attack_otf(U64 occ, int sq)
{
    U64 att = 0;
    int rank = sq / 8, file = sq % 8, r, f;

    for (r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    for (r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    for (r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    return att;
}

U64 pin_bishop_attack_otf(U64 occ, int sq)
{
    U64 att = 0;
    int rank = sq / 8, file = sq % 8, r, f;
    bool skip_flag = false;

    for (r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }
    skip_flag = false;

    for (r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }
    skip_flag = false;

    for (r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }
    skip_flag = false;

    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }

    return att;
}

U64 rook_attack_otf(U64 occ, int sq)
{
    U64 att = 0;
    int rank = sq / 8, file = sq % 8, r, f;

    for (r = rank + 1, f = file; r < 8; r++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    for (r = rank, f = file + 1; f < 8; f++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    for (r = rank - 1, f = file; r >= 0; r--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    for (r = rank, f = file - 1; f >= 0; f--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            break;
        }
    }

    return att;
}

U64 pin_rook_attack_otf(U64 occ, int sq)
{
    U64 att = 0;
    int rank = sq / 8, file = sq % 8, r, f;
    bool skip_flag = false;

    for (r = rank + 1, f = file; r < 8; r++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }
    skip_flag = false;

    for (r = rank, f = file + 1; f < 8; f++)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }
    skip_flag = false;

    for (r = rank - 1, f = file; r >= 0; r--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }
    skip_flag = false;

    for (r = rank, f = file - 1; f >= 0; f--)
    {
        SET_BIT(att, r * 8 + f);
        if (GET_BIT(occ, r * 8 + f))
        {
            if(skip_flag)
            {
                break;
            }
            else
            {
                skip_flag = true;
            }
        }
    }

    return att;
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
    U64 occupancy = 0ULL;

    // loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++)
    {
        // get next bit index and pop it
        int square = bit_index(attack_mask);
        CLEAR_BIT(attack_mask, square);

        // index here is used as a bit mask to generate different
        // occupancies
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }

    return occupancy;
}

unsigned int xorshift32_seed = 1916057908;

// Custom XOR shift pseudo-random number generator function
unsigned int xorshift32()
{
    xorshift32_seed ^= (xorshift32_seed << 13);
    xorshift32_seed ^= (xorshift32_seed >> 17);
    xorshift32_seed ^= (xorshift32_seed << 5);
    return xorshift32_seed;
}

U64 random_U64()
{
    U64 b1, b2, b3, b4;

    b1 = (U64)(xorshift32()) & 0xFFFF;
    b2 = (U64)(xorshift32()) & 0xFFFF;
    b3 = (U64)(xorshift32()) & 0xFFFF;
    b4 = (U64)(xorshift32()) & 0xFFFF;

    return b1 | (b2 << 16) | (b3 << 32) | (b4 << 48);
}

U64 random_U64_little()
{
    return random_U64() & random_U64() & random_U64();
}

U64 find_bishop_magic(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U64 (&magic)[SIZE], U64 (&mask)[SIZE], int sq, U8 relevant_bits, U64 (&on_the_fly_function)(U64, int))
{

    // variables preparation
    U64 occupancies[512];
    U64 attacks[512];
    U64 used_attacks[512];
    U64 mask_attack = mask[sq];
    int occupancy_variations = 1 << relevant_bits;

    // loop over the number of occupancy variations
    for (int count = 0; count < occupancy_variations; count++)
    {
        // init occupancies
        occupancies[count] = set_occupancy(count, relevant_bits, mask_attack);

        // init attacks
        attacks[count] = on_the_fly_function(occupancies[count], sq);
    }

    // ** magic **
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        // init magic number candidate
        U64 magic_num = random_U64_little();

        /*
        EXPLANATION!!
        we multiply the magic with the biggest possible mask to get the biggest
        possible magic number. By calculating bitwise and of the magic number
        and 8 most significant bits (knowing that the next operation is the bit
        shift >> 64-relevant_bits) we can estimate number of possible magic indexes,
        which with count == 5 is too low. thats why we set a boud to 6 >=.
        */
        if (count_bits((mask_attack * magic_num) & 0xFF00000000000000ULL) < 6)
        {
            continue;
        }

        // init vars for search
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        int count, fail;

        // test magic index
        for (count = 0, fail = 0; !fail && count < occupancy_variations; count++)
        {
            // generate magic index
            int magic_index = (int)((occupancies[count] * magic_num) >> (64 - relevant_bits));

            // if got free index
            if (used_attacks[magic_index] == 0ULL)
                // assign corresponding attack map
                used_attacks[magic_index] = attacks[count];

            // otherwise fail on  collision
            else if (used_attacks[magic_index] != attacks[count])
                fail = 1;
        }

        // return magic if it works
        if (!fail) {
            magic[sq] = magic_num;
            // save used_attacks under attack table
            memcpy(attack[sq], used_attacks, sizeof(used_attacks));
            return magic_num;
        }
    }

    // on fail
    printf("***Failed***\n");
    return 0ULL;
}

U64 find_rook_magic(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], U64 (&magic)[SIZE], U64 (&mask)[SIZE], int sq, U8 relevant_bits, U64 (&on_the_fly_function)(U64, int))
{

    // variables preparation
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];
    U64 mask_attack = mask[sq];
    int occupancy_variations = 1 << relevant_bits;

    // loop over the number of occupancy variations
    for (int count = 0; count < occupancy_variations; count++)
    {
        // init occupancies
        occupancies[count] = set_occupancy(count, relevant_bits, mask_attack);

        // init attacks
        attacks[count] = on_the_fly_function(occupancies[count], sq);
    }

    // ** magic **
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        // init magic number candidate
        U64 magic_num = random_U64_little();

        /*
        EXPLANATION!!
        we multiply the magic with the biggest possible mask to get the biggest
        possible magic number. By calculating bitwise and of the magic number
        and 8 most significant bits (knowing that the next operation is the bit
        shift >> 64-relevant_bits) we can estimate number of possible magic indexes,
        which with count == 5 is too low. thats why we set a boud to 6 >=.
        */
        if (count_bits((mask_attack * magic_num) & 0xFF00000000000000ULL) < 6)
        {
            continue;
        }

        // init vars for search
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        int count, fail;

        // test magic index
        for (count = 0, fail = 0; !fail && count < occupancy_variations; count++)
        {
            // generate magic index
            int magic_index = (int)((occupancies[count] * magic_num) >> (64 - relevant_bits));

            // if got free index
            if (used_attacks[magic_index] == 0ULL)
                // assign corresponding attack map
                used_attacks[magic_index] = attacks[count];

            // otherwise fail on  collision
            else if (used_attacks[magic_index] != attacks[count])
                fail = 1;
        }

        // return magic if it works
        if (!fail) {
            magic[sq] = magic_num;
            memcpy(attack[sq], used_attacks, sizeof(used_attacks));
            return magic_num;
        }
    }

    // on fail
    printf("***Failed***\n");
    return 0ULL;
}

void print_array_64(U64 (&arr)[SIZE], const char* name) {
    printf("\n\nconstexpr U64 ");
    printf("%s", name);
    printf("[SIZE] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < SIZE; square++)
        printf("0x%lxUL,\n", arr[square]);

    printf("};\n\n");
}

void print_array_8(U8 (&arr)[SIZE], const char* name) {
    printf("\n\nconstexpr U8 ");
    printf("%s", name);
    printf("[SIZE] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < SIZE; square++)
        printf("%d,\n", arr[square]);

    printf("};\n\n");
}

void save_attack_bishop(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], const char* filename) {

    FILE* outf = fopen(filename, "w");
    if( NULL == outf ) {
        printf("File open fail\n");
        return;
    }

    fprintf(outf, "\n\nconstexpr U64 bishop_attack[%d][%d] = {\n", SIZE, BISHOP_ATTACK_SIZE);

    // loop over 64 board squares
    for (int square = 0; square < SIZE; square++)
    {
        for (size_t attack_index = 0; attack_index < BISHOP_ATTACK_SIZE; attack_index++)
        {
            fprintf(outf, "0x%lxUL,", attack[square][attack_index]);
        }
        fprintf(outf, "\n");
    }
    fprintf(outf, "};\n\n");
    fclose(outf);
}

void save_attack_rook(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], const char* filename) {

    FILE* outf = fopen(filename, "w");
    if( NULL == outf ) {
        printf("File open fail\n");
        return;
    }

    fprintf(outf, "#include \"common/types.h\"\n");
    fprintf(outf, "\nconstexpr U64 rook_attack[%d][%d] = {\n", SIZE, ROOK_ATTACK_SIZE);

    // loop over 64 board squares
    for (int square = 0; square < SIZE; square++)
    {
        for (size_t attack_index = 0; attack_index < ROOK_ATTACK_SIZE; attack_index++)
        {
            fprintf(outf, "0x%lxUL,", attack[square][attack_index]);
        }
        fprintf(outf, "\n");
    }
    fprintf(outf, "};\n\n");
    fclose(outf);
}

void generate_bishop_attack(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE])
{

    // fill relevant_bits and mask arrays
    for (U8 sq = 0; sq < 64; sq++)
    {
        mask[sq] = make_bishop_mask(sq);
        relevant_bits[sq] = count_bits(mask[sq]);
    }

    printf("\n\nconstexpr U64 bishop_magic[SIZE] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        printf("    0x%lxUL,\n", find_bishop_magic(attack, magic, mask, square, relevant_bits[square], bishop_attack_otf));

    printf("};\n\n");

    // print_array_64(mask, "bishop_mask");
    // print_array_8(relevant_bits, "bishop_relevant_bits");
    // save_attack_bishop(attack, "attack_tables_data_bishop.h");
}

void generate_rook_attack(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE])
{

    // fill relevant_bits and mask arrays
    for (U8 sq = 0; sq < 64; sq++)
    {
        mask[sq] = make_rook_mask(sq);
        relevant_bits[sq] = count_bits(mask[sq]);
    }

    printf("\n\nconstexpr U64 rook_magic[SIZE] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        printf("    0x%lxUL,\n", find_rook_magic(attack, magic, mask, square, relevant_bits[square], rook_attack_otf));

    printf("};\n\n");

    // print_array_64(mask, "rook_mask");
    // print_array_8(relevant_bits, "rook_relevant_bits");
    // save_attack_rook(attack, "attack_tables_data_rook.h");
}

void generate_bishop_pin_attack(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE])
{

    // fill relevant_bits and mask arrays
    for (U8 sq = 0; sq < 64; sq++)
    {
        mask[sq] = make_bishop_mask(sq);
        relevant_bits[sq] = count_bits(mask[sq]);
    }

    printf("\n\nconstexpr U64 bishop_magic_pin[SIZE] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        printf("    0x%lxUL,\n", find_bishop_magic(attack, magic, mask, square, relevant_bits[square], pin_bishop_attack_otf));

    printf("};\n\n");

    // print_array_64(mask, "bishop_mask");
    // print_array_8(relevant_bits, "bishop_relevant_bits");
    save_attack_bishop(attack, "attack_tables_data_bishop_pin.h");
}

void generate_rook_pin_attack(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE])
{

    // fill relevant_bits and mask arrays
    for (U8 sq = 0; sq < 64; sq++)
    {
        mask[sq] = make_rook_mask(sq);
        relevant_bits[sq] = count_bits(mask[sq]);
    }

    printf("\n\nconstexpr U64 rook_magic_pin[SIZE] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        printf("    0x%lxUL,\n", find_rook_magic(attack, magic, mask, square, relevant_bits[square], pin_rook_attack_otf));

    printf("};\n\n");

    // print_array_64(mask, "rook_mask");
    // print_array_8(relevant_bits, "rook_relevant_bits");
    save_attack_rook(attack, "attack_tables_data_rook_pin.h");
}

/* -------------------------------------------------------------------------- */
/*                        CHECKMASKS LOOKUP GENERATION                        */
/* -------------------------------------------------------------------------- */

void checkmask_print(U64 (&attack)[SIZE][SIZE], const char* name)
{
    printf("constexpr U64 %s[%d][%d] = {\n", name, SIZE, SIZE);

    // loop over 64 board squares
    for (int square = 0; square < SIZE; square++)
    {
        for (int attack_index = 0; attack_index < SIZE; attack_index++)
        {
            fprintf(stdout, "0x%lxUL,", attack[square][attack_index]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "};\n\n");
}

void generate_rook_checkmask(U64 (&attack)[SIZE][SIZE])
{
    bool found_flag = false;
    int cindex = 0;

    // iterate through all pieces
    for(int king_idx=0; king_idx<SIZE; king_idx++)
    {
        for(int piece_idx=0; piece_idx<SIZE; piece_idx++)
        {
            //iterate through all directions -> if found save and break

            // upward
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(int row=(king_idx/8) + 1; row<=(piece_idx/8); row++){

                cindex = row*8 + (king_idx % 8);

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            // downward
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(int row=(king_idx/8) - 1; row>=(piece_idx/8); row--){

                cindex = row*8 + (king_idx % 8);

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            // left
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(int col=(king_idx % 8) + 1; col<=(piece_idx % 8); col++){

                cindex = (king_idx / 8)*8 + col;

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            // right
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(int col=(king_idx % 8) - 1; col>=(piece_idx % 8); col--){

                cindex = (king_idx / 8)*8 + col;

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            attack[king_idx][piece_idx] = 0;
        }
    }
    // checkmask_print(attack, "rook_checkmask");
}

void generate_bishop_checkmask(U64 (&attack)[SIZE][SIZE])
{
    bool found_flag = false;
    int cindex = 0, row, col;

    // iterate through all pieces
    for(int king_idx=0; king_idx<SIZE; king_idx++)
    {
        for(int piece_idx=0; piece_idx<SIZE; piece_idx++)
        {
            //iterate through all directions -> if found save and break

            // upward - right
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(row=(king_idx/8) + 1, col=(king_idx % 8) + 1; row<=(piece_idx/8) && col<=(piece_idx % 8); row++, col++){

                cindex = row*8 + col;

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            // downward - right
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(row=(king_idx/8) - 1, col=(king_idx % 8) + 1; row>=(piece_idx/8) && col<=(piece_idx % 8); row--, col++){

                cindex = row*8 + col;

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            // downward - left
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(row=(king_idx/8) - 1, col=(king_idx % 8) - 1; row>=(piece_idx/8) && col>=(piece_idx % 8); row--, col--){

                cindex = row*8 + col;

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }

            // upward - left
            attack[king_idx][piece_idx] = 0;
            found_flag = false;
            for(row=(king_idx/8) + 1, col=(king_idx % 8) - 1; row<=(piece_idx/8) && col>=(piece_idx % 8); row++, col--){

                cindex = row*8 + col;

                SET_BIT(attack[king_idx][piece_idx], cindex);

                if(cindex == piece_idx) {
                    found_flag = true;
                }
            }
            if(found_flag)
            {
                continue;
            }
        }
    }
    // checkmask_print(attack, "bishop_checkmask");
}

void calc_sadsquare(int king_idx, int piece_idx, int &ss_file, int &ss_rank)
{
    if(piece_idx == 64 || king_idx == piece_idx)
    {
        ss_rank = 7;
        ss_file = 8;
        // idx = 64
        return;
    }
    int king_rank = king_idx/8, king_file = (king_idx % 8), piece_rank = piece_idx/8, piece_file = (piece_idx % 8);
    if(king_rank == piece_rank && piece_file > king_file)
    {
        ss_rank = king_rank;
        ss_file = piece_file - 2;
    }
    else if(king_rank == piece_rank && king_file > piece_file)
    {
        ss_rank = king_rank;
        ss_file = piece_file + 2;
    }
    else if(king_file == piece_file && king_rank > piece_rank)
    {
        ss_rank = piece_rank + 2;
        ss_file = king_file;
    }
    else if(king_file == piece_file && piece_rank > king_rank)
    {
        ss_rank = piece_rank - 2;
        ss_file = king_file;
    }
    else if(king_rank == piece_rank + 1 && king_file == piece_file + 1)
    {
        ss_rank = king_rank + 1;
        ss_file = king_file + 1;
    }
    else if(king_rank == piece_rank - 1 && king_file == piece_file + 1)
    {
        ss_rank = king_rank - 1;
        ss_file = king_file + 1;
    }
    else if(king_rank == piece_rank + 1 && king_file == piece_file - 1)
    {
        ss_rank = king_rank + 1;
        ss_file = king_file - 1;
    }
    else if(king_rank == piece_rank - 1 && king_file == piece_file - 1)
    {
        ss_rank = king_rank - 1;
        ss_file = king_file - 1;
    }
    else
    {
        ss_rank = 7;
        ss_file = 8;
        // idx = 64
    }
}

void sadsquares_print(U64 (&attack)[SIZE][SIZE+1])
{
    printf("constexpr U64 sadsquare[%d][%d] = {\n", SIZE, SIZE+1);

    // loop over 64 board squares
    for (int square = 0; square < SIZE; square++)
    {
        for (int attack_index = 0; attack_index < SIZE+1; attack_index++)
        {
            fprintf(stdout, "0x%lxUL,", attack[square][attack_index]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "};\n\n");
}

void generate_sadsquares(U64 (&arr)[SIZE][SIZE + 1])
{
    int ss_file, ss_rank;

    for(int king_idx=0; king_idx<SIZE; king_idx++)
    {
        for(int piece_idx=0; piece_idx<SIZE+1; piece_idx++)
        {
            arr[king_idx][piece_idx] = 0;

            calc_sadsquare(king_idx, piece_idx, ss_file, ss_rank);
            if(ss_file >= 0 && ss_file < 8 && ss_rank >= 0 && ss_rank < 8)
            {
                arr[king_idx][piece_idx] = 1UL << (ss_rank*8 + ss_file);
            }
        }
    }

    // sadsquares_print(arr);
}

#endif