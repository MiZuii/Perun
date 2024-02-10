#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "../search/search.h"

class Game
{
private:

    static Board _board;
    static std::vector<Move_t> _move_history;
    static SearchArgs _search_args;
    static std::jthread _search_thread;
    static EngineResults _engine_results;
    static std::mutex _engine_results_mutex;

protected:
    
    Game();
    ~Game();
    static Game* _instance;

public:

    Game(Game &other) = delete;
    Game(Game &&other) = delete;
    void operator=(const Game &other) = delete;

    static Game* getInstance();
    static void destroy();

    // main methods for game reseting.
    static void clear();
    static void create();
    static void setPosition(FEN_t fen); // setPosition resets the moves
    static void setMoves(std::vector<Move_t> moves);
    static void setSearchArgs(SearchArgs args);
    static void start();
    static void stop();
    static EngineResults &getEngineResultsStruct();
    static std::mutex &getEngineResultsMutex();
    static void cleanEngineResults();

    static bool game_initialized;
    static bool game_running;

};
