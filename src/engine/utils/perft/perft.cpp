#include "perft.h"

void _perft(Board board, int depth, U32 &counter)
{
    if(depth <= 0)
    {
        counter++;
        return;
    }

    Board next_board;
    std::vector<Move_t> moves = board.getMoves();

    for(Move_t move : moves)
    {
        next_board = board;
        next_board.makeMove(move);
        _perft(next_board, depth-1, counter);
    }
}

void perft(FEN_t fen, int depth)
{
    std::cout << "Performing perft for position: " << fen << "\n" << std::endl;

    U32 count = 0;
    U32 sub_count = 0;
    int64_t time = 0;
    double speed = 0;
    Board b(fen), bc;

    // start clock
    auto start = std::chrono::high_resolution_clock::now();

    // first generate all moves for depth 0
    std::vector<Move_t> moves = b.getMoves();


    for(Move_t move : moves)
    {
        sub_count = 0;
        bc = b;
        bc.makeMove(move);
        _perft(bc, depth-1, sub_count);
        std::cout << Board::moveToStringShort(move) << " " << sub_count << std::endl;
        count += sub_count;
    }

    // stop clock
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - start);
    time = duration.count();
    speed = (count*0.001)/time; //(double)count*(1/1000))/(double)time

    std::cout << "\nFinal perft result: " << count << std::endl;
    std::cout << "Completed in " << time << "ms - " << std::setprecision(2) << speed << "Mn/s" << std::endl;
}