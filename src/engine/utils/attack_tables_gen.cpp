#include "common/includes.h"
#include "common/types.h"
#include "common/bit_opers.h"

#include "attack_tables_gen.h"

#if 1

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

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
    U64 occupancy = 0ULL;

    // loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++)
    {
        // get next bit index and pop it
        int square = log2_64(LS1B(attack_mask));
        CLEAR_BIT(attack_mask, square);

        // index here is used as a bit mask to generate different
        // occupancies
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }

    return occupancy;
}

unsigned int xorshift32_seed = 56785678678324;

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

U64 find_bishop_magic(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U64 (&magic)[SIZE], U64 (&mask)[SIZE], int sq, U8 relevant_bits)
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
        attacks[count] = bishop_attack_otf(occupancies[count], sq);
    }

    // ** magic **
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        // init magic number candidate
        U64 magic = random_U64_little();

        /*
        EXPLANATION!!
        we multiply the magic with the biggest possible mask to get the biggest
        possible magic number. By calculating bitwise and of the magic number
        and 8 most significant bits (knowing that the next operation is the bit
        shift >> 64-relevant_bits) we can estimate number of possible magic indexes,
        which with count == 5 is too low. thats why we set a boud to 6 >=.
        */
        if (count_bits((mask_attack * magic) & 0xFF00000000000000ULL) < 5)
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
            int magic_index = (int)((occupancies[count] * magic) >> (64 - relevant_bits));

            // if got free index
            if (used_attacks[magic_index] == 0ULL)
                // assign corresponding attack map
                used_attacks[magic_index] = attacks[count];

            // otherwise fail on  collision
            else if (used_attacks[magic_index] != attacks[count])
                fail = 1;
        }

        // return magic if it works
        if (!fail)
        
            return magic;
    }

    // on fail
    printf("***Failed***\n");
    return 0ULL;
}

void generate_bishop_attack(U64 (&attack)[SIZE][BISHOP_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE])
{

    // fill relevant_bits and mask arrays
    for (U8 sq = 0; sq < 64; sq++)
    {
        mask[sq] = make_bishop_mask(sq);
        relevant_bits[sq] = count_bits(mask[sq]);
    }

    printf("};\n\nconst U64 bishop_magics[64] = {\n");

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        printf("    0x%llxULL,\n", find_bishop_magic(attack, magic, mask, square, relevant_bits[square]));

    printf("};\n\n");
}

void generate_rook_attack(U64 (&attack)[SIZE][ROOK_ATTACK_SIZE], U8 (&relevant_bits)[SIZE], U64 (&mask)[SIZE], U64 (&magic)[SIZE])
{
}

#endif