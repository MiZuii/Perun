#include "game.h"

Game::Game() {}

Game *Game::getInstance()
{
    if( nullptr == _instance ){
        _instance = new Game();
    }
    return _instance;
}

Game *Game::newGame(const FEN_t starting_position)
{
    _board = Board(starting_position);
    _move_history.clear();

    return Game::getInstance();
}

Game *Game::newGame(const std::vector<Move_t> moves_history, const FEN_t starting_position)
{
    _board = Board(starting_position);
    _move_history = moves_history;

    return Game::getInstance();
}
