#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "board_repr.h"

#if DEBUG

std::string BitBoardBase::toString() const
{
    std::string ret;

    for (int i = 7; i > -1; i--)
    {
        ret += std::to_string(i+1);
        
        for (int j = 7; j > -1; j--)
        {

            if( GET_BIT(this->_bit_board, (i * 8 + j)) ) {
                ret += " 1";
            } else {
                ret += " 0";
            }
        }
        ret += "\n";
    }

    ret += "  a b c d e f g h\n";

    return ret;
}

std::string PureBitBoard::toString() const
{
    return this->BitBoardBase::toString();
}

std::string PieceBitBoard::toString() const
{
    return this->BitBoardBase::toString();
}

std::ostream &operator<<(std::ostream &os, const PieceBitBoard &obj)
{
    os << obj.toString();
    return os;
}

std::ostream &operator<<(std::ostream &os, const PureBitBoard &obj)
{
    os << obj.toString();
    return os;
}

#endif

BitBoardBase::~BitBoardBase() {}

PureBitBoard::PureBitBoard(U64 board)
{
    _bit_board = board;
}

PieceBitBoard::PieceBitBoard(U64 board, Piece_t piece) : _piece(piece)
{
    _bit_board = board;
}
