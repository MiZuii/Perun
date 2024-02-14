#include "engine/utils/common/includes.h"
#include "engine/utils/common/types.h"
#include "engine/utils/common/bit_opers.h"

#include "engine/utils/perft/perft.h"

#include "engine/interface/interface.h"

#include "engine/utils/attack_tables/attack_tables_gen.h"

int main(void) {
#if DEBUG

    // perft(STARTING_POS, 4);

    // std::vector<Move_t> moves{34363026, 35436195, 34363611, 825458, 35412643, 824481, 34364586, 9202347, 33644210, 9201834, 1841338};
    // std::vector<Move_t> moves{34363026, 35436195, 34363611, 825458, 35412643, 824481, 34364586, 9202475, 33644210, 9202028, 1841530};
    // for(auto move : moves)
    // {
    //     std::cout << Board::moveToString(move) << std::endl;
    // }

    // Board b{"5Q2/8/3k4/8/5p2/5n2/5K2/8 b - - 0 5"};
    // b.getMoves();
    // for(auto move : b.moves)
    // {
    //     std::cout << Board::moveToString(move) << std::endl;
    // }

    run();

#endif
    return 0;
}