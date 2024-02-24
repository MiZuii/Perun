#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/bit_opers.h"
#include "../utils/common/types.h"

#include "../board/board_repr.h"

// piece_hash_keys[piece][square]
_Inline U64 piece_hash_keys[12][64];

// en_passant_hash_keys[en_passant square]
_Inline U64 en_passant_hash_keys[65];
/* the 65 key is 0 for reducing branching on move make function*/

// castling_hash_keys[all possible castle combinations]
_Inline U64 castling_hash_keys[16];

// added if side is white
_Inline U64 side_hash_key;

void fill_hash_arrays();

/* -------------------------------------------------------------------------- */
/*                             TRANSPOSITION TABLE                            */
/* -------------------------------------------------------------------------- */

struct TTItem
{
    U64         key;
    ScoreVal_t  score;
    Depth_t     depth;
    U8          pv   : 1,
                type : 2,
                gen  : 5;
    Move_t      bm;

    friend class TT;
};

class TT
{
private:

    static constexpr size_t ITEM_SIZE = sizeof(TTItem);
    static inline TTItem *_tt = nullptr;
    static inline size_t _size = 400000;

public:

    static void         init();
    static void         resize(size_t new_size);
    static void         destroy();
    static ScoreVal_t   probe(Board &board, ScoreVal_t alpha, ScoreVal_t beta, Depth_t gen);
    static TTItem*      raw_probe(Board &board, Depth_t gen);
    static void         write(Board &board, ScoreVal_t score, NodeType nt, Move_t bm, Depth_t gen);
    static void         pv_probe(Board board, Depth_t gen, std::vector<Move_t> &pvv);

};


/* -------------------------------------------------------------------------- */
/*                           RANDOM NUBER GENERATOR                           */
/* -------------------------------------------------------------------------- */

class XORSHIFT
{
private:
    inline static uint64_t x = 0;

public:
    static void init(uint64_t seed)
    {
        assert(seed != 0);
        x = seed;
    }

    static uint64_t get() {
        x ^= x >> 12; // a
        x ^= x << 25; // b
        x ^= x >> 27; // c
        return x * UINT64_C(2685821657736338717);
    }
};