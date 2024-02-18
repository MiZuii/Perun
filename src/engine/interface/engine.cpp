#include "engine.h"

Engine* Engine::_instance = nullptr;
Board Engine::_board;
std::vector<Move_t> Engine::_move_history;
bool Engine::engine_initialized = false;
bool Engine::engine_running = false;
std::jthread Engine::_search_thread;
SearchArgs Engine::_search_args;
EngineResults Engine::_engine_results;
std::mutex Engine::_engine_results_mutex;

Engine::Engine()
{

}

Engine::~Engine()
{
    if(_search_thread.joinable())
    {
        _search_thread.request_stop();
        _search_thread.join();
    }
}

Engine *Engine::getInstance()
{
    if( nullptr == _instance ){
        _instance = new Engine();
    }
    return _instance;
}

void Engine::destroy()
{
    if( nullptr == _instance )
    {
        return;
    }
    delete _instance;
}

void Engine::clear()
{
    engine_initialized = false;
    engine_running = false;

    // todo
}

void Engine::create()
{
    assert(!engine_initialized);
    assert(!engine_running);

    engine_initialized = true;

    // todo
}

void Engine::setPosition(FEN_t fen)
{
    assert(engine_initialized);
    assert(!engine_running);

    _move_history.clear();
    _board = Board(fen);
}

void Engine::setMoves(std::vector<Move_t> moves)
{
    assert(engine_initialized);
    assert(!engine_running);

    for(Move_t move : moves)
    {
        _board.makeMove(move);
    }

    _move_history = moves;
}

void Engine::setSearchArgs(SearchArgs args)
{
    _search_args = args;
}

void Engine::start()
{
    if(_search_thread.joinable())
    {
        _search_thread.join();
    }

    cleanEngineResults();

    _search_thread = std::jthread(search,
        _board, _move_history, _search_args, std::ref(_engine_results), std::ref(_engine_results_mutex));
    engine_running = true;
}

void Engine::stop()
{
    _search_thread.request_stop();
}

EngineResults &Engine::getEngineResultsStruct()
{
    return _engine_results;
}

std::mutex &Engine::getEngineResultsMutex()
{
    return _engine_results_mutex;
}

void Engine::cleanEngineResults()
{
    _engine_results.finished = false;

    _engine_results.best_move = 0;
    _engine_results.best_score = 0;
    _engine_results.current_max_depth = 0;
    
    _engine_results.node_count.store(0);
}
