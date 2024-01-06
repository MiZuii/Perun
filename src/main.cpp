#include "engine/utils/common/includes.h"
#include "engine/utils/common/types.h"
#include "engine/utils/common/bit_opers.h"

#include "engine/utils/attack_tables/attack_tables.h"
#include "engine/utils/attack_tables/attack_tables_gen.h"
#include "engine/board/board_repr.h"

int main(void) {
#if DEBUG

    std::vector<U16> moves;
    // wcout configuration
    // constexpr char locale_name[] = "";
    // setlocale( LC_ALL, locale_name );
    // std::locale::global(std::locale(locale_name));
    // std::wcin.imbue(std::locale());
    // std::wcout.imbue(std::locale());

    // Board test1("8/2r3b1/8/q7/1NPP4/1BKN3r/4R3/8 w KQkq - 0 1");
    Board test1("8/2r3b1/3K4/8/8/5Q2/8/8 w KQkq - 0 1");
    // std::cout << b.toString();
    // std::wcout << b.toWString();
    // std::cout << b.getFEN();

    moves = test1.getMoves();
    std::cout << "test1 moves:" << std::endl;
    for(auto move : moves)
    {
        std::cout << Board::moveToString(move) << std::endl;
    }

#endif
    return 0;
}