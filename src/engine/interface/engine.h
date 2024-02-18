#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "../search/search.h"

class Engine
{
private:

    static Board _board;
    static std::vector<Move_t> _move_history;
    static SearchArgs _search_args;
    static std::jthread _search_thread;
    static EngineResults _engine_results;
    static std::mutex _engine_results_mutex;

protected:
    
    Engine();
    ~Engine();
    static Engine* _instance;

public:

    Engine(Engine &other) = delete;
    Engine(Engine &&other) = delete;
    void operator=(const Engine &other) = delete;

    static Engine* getInstance();
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

    static bool engine_initialized;
    static bool engine_running;

};
