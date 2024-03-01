#include "engine/interface/interface.h"

int main(void) {

    //rnbqkbnr/pppp1pp1/8/4P1Bp/2B1P3/2N2N2/PPP2PPP/R2QK2R b KQkq - 2 7
    //rnbqkbnr/pppp1ppp/8/4p3/7P/4P3/PPPP1PP1/RNBQKBNR b KQkq h3 0 2

    std::atexit(TT::destroy);
    std::atexit(Engine::destroy);

    Engine::init();
    TT::init();

    run();

    return 0;
}