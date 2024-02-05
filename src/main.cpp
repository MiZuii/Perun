#include "engine/utils/common/includes.h"
#include "engine/utils/common/types.h"
#include "engine/utils/common/bit_opers.h"

#include "engine/utils/attack_tables/attack_tables.h"
#include "engine/utils/attack_tables/attack_tables_gen.h"
#include "engine/board/board_repr.h"

#include "engine/utils/perft/perft.h"

int main(void) {
#if DEBUG

    std::vector<Move_t> moves;
    // wcout configuration
    // constexpr char locale_name[] = "";
    // setlocale( LC_ALL, locale_name );
    // std::locale::global(std::locale(locale_name));
    // std::wcin.imbue(std::locale());
    // std::wcout.imbue(std::locale());

    // std::cout << b.toString();
    // std::wcout << b.toWString();

    perft(STARTING_POS, 6);

    // Board test1;
    // moves = test1.getMoves();
    // std::cout << "test1 moves:" << std::endl;
    // for(auto move : moves)
    // {
    //     std::cout << Board::moveToString(move) << std::endl;
    //     Board subtest = test1;
    //     subtest.makeMove(move);
    //     std::cout << subtest.toString() << std::endl;
    // }

#endif
    return 0;
}