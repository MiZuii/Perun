#include "game.h"

Game* Game::_instance = nullptr;
Board Game::_board;
std::vector<Move_t> Game::_move_history;
bool Game::game_initialized = false;
bool Game::game_running = false;
std::jthread Game::_search_thread;
SearchArgs Game::_search_args;
EngineResults Game::_engine_results;
std::mutex Game::_engine_results_mutex;

Game::Game()
{

}

Game::~Game()
{
    _search_thread.request_stop();
    _search_thread.join();
}

Game *Game::getInstance()
{
    if( nullptr == _instance ){
        _instance = new Game();
    }
    return _instance;
}

void Game::destroy()
{
    if( nullptr == _instance )
    {
        return;
    }
    delete _instance;
}

void Game::clear()
{
    game_initialized = false;
    game_running = false;

    // todo
}

void Game::create()
{
    assert(!game_initialized);
    assert(!game_running);

    game_initialized = true;

    // todo
}

void Game::setPosition(FEN_t fen)
{
    assert(game_initialized);
    assert(!game_running);

    _move_history.clear();
    _board = Board(fen);
}

void Game::setMoves(std::vector<Move_t> moves)
{
    assert(game_initialized);
    assert(!game_running);

    for(Move_t move : moves)
    {
        _board.makeMove(move);
    }

    _move_history = moves;
}

void Game::setSearchArgs(SearchArgs args)
{
    _search_args = args;
}

void Game::start()
{
    if(_search_thread.joinable())
    {
        _search_thread.join();
    }

    cleanEngineResults();

    _search_thread = std::jthread(search,
        _board, _move_history, _search_args, std::ref(_engine_results), std::ref(_engine_results_mutex));
    game_running = true;
}

void Game::stop()
{
    _search_thread.request_stop();
}

EngineResults &Game::getEngineResultsStruct()
{
    return _engine_results;
}

std::mutex &Game::getEngineResultsMutex()
{
    return _engine_results_mutex;
}

void Game::cleanEngineResults()
{
    _engine_results.finished = false;
    _engine_results.new_data = false;
}
