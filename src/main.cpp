#include "engine/utils/common/includes.h"
#include "engine/utils/common/types.h"
#include "engine/utils/common/bit_opers.h"

#include "engine/utils/attack_tables/attack_tables.h"
#include "engine/board/board_repr.h"

#include "engine/utils/perft/perft.h"

int main(void) {
#if DEBUG

    // wcout configuration
    // constexpr char locale_name[] = "";
    // setlocale( LC_ALL, locale_name );
    // std::locale::global(std::locale(locale_name));
    // std::wcin.imbue(std::locale());
    // std::wcout.imbue(std::locale());

    // std::cout << b.toString();
    // std::wcout << b.toWString();

    perft(STARTING_POS, 6);

#endif
    return 0;
}