#include "engine/utils/common/includes.h"
#include "engine/utils/common/types.h"
#include "engine/utils/common/bit_opers.h"

#include "engine/utils/attack_tables/attack_tables.h"
#include "engine/utils/attack_tables/attack_tables_gen.h"
#include "engine/board/board_repr.h"

#include <chrono>

int main(void) {
#if DEBUG

    std::vector<U16> moves;
    // wcout configuration
    // constexpr char locale_name[] = "";
    // setlocale( LC_ALL, locale_name );
    // std::locale::global(std::locale(locale_name));
    // std::wcin.imbue(std::locale());
    // std::wcout.imbue(std::locale());

    // std::cout << b.toString();
    // std::wcout << b.toWString();

    // Board test1("8/2r3b1/8/q7/1NPP4/1BKN3r/4R3/8 w KQkq - 0 1");
    
    // perft XD?15834ms
    auto start = std::chrono::high_resolution_clock::now();
    Board test1;
    for (size_t i = 0; i < 119060324; i++)
    {
        test1.getMoves();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pseudo perft result: " << duration.count() << "ms" << std::endl;

    // moves = test1.getMoves();
    // std::cout << "test1 moves:" << std::endl;
    // for(auto move : moves)
    // {
    //     std::cout << Board::moveToString(move) << std::endl;
    // }

#endif
    return 0;
}