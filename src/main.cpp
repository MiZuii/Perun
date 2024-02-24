#include "engine/interface/interface.h"

#include "engine/utils/perft/perft.h"

int main(void) {

    std::atexit(TT::destroy);
    std::atexit(Engine::destroy);

    Engine::init();
    TT::init();

    run();

    // perft(STARTING_POS, 6);
    return 0;
}