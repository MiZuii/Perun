#pragma once

#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "../board/board_repr.h"

class Game
{
private:

    static Board _board;
    static std::vector<Move_t> _move_history;

protected:
    
    Game();
    static Game* _instance;

public:

    Game(Game &other) = delete;
    Game(Game &&other) = delete;
    void operator=(const Game &other) = delete;

    static Game* getInstance();

    static Game* newGame(const FEN_t starting_position = STARTING_POS);
    static Game* newGame(const std::vector<Move_t> moves_history, const FEN_t starting_position = STARTING_POS);

};

Game* Game::_instance = nullptr;
Board Game::_board;
std::vector<Move_t> Game::_move_history;
