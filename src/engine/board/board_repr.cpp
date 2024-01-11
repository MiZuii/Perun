#include "../utils/common/includes.h"
#include "../utils/common/types.h"
#include "../utils/common/bit_opers.h"

#include "board_repr.h"

#if DEBUG

BitBoardWrap::BitBoardWrap() : _bit_board(0) {}

BitBoardWrap::BitBoardWrap(U64 board) : _bit_board(board) {}

std::string BitBoardWrap::bitboard_repr(U64 bb)
{
    return BitBoardWrap(bb).toString();
}

std::string BitBoardWrap::toString() const
{
    std::string ret;

    for (int i = 7; i > -1; i--)
    {
        ret += std::to_string(i + 1);

        for (int j = 7; j > -1; j--)
        {

            if (GET_BIT(this->_bit_board, (i * 8 + j)))
            {
                ret += " 1";
            }
            else
            {
                ret += " 0";
            }
        }
        ret += "\n";
    }

    ret += "  a b c d e f g h\n";

    return ret;
}

std::ostream &operator<<(std::ostream &os, const BitBoardWrap &obj)
{
    os << obj.toString();
    return os;
}

#endif

Board::Board() : Board(STARTING_POS) {}

Board::Board(FEN_t fen)
{

    // fen validation
    if(!validFEN(fen))
    {
        std::cerr << "Invalid fen in Board constructor. Starting position used instead" << std::endl;
        fen = STARTING_POS;
    }    

    std::istringstream iss(fen);
    std::istringstream *sub_iss;
    std::vector<std::string> tokens, row_tokens;
    std::string token, row_token;
    int tokeni = 0, row_tokeni = 0, coli = 0;

    while (std::getline(iss, token, ' '))
    {
        tokens.push_back(token);
    }

    for (std::string token : tokens)
    {

        switch (tokeni)
        {
        case 0:
            /* main board fillup */
            sub_iss = new std::istringstream(token);
            while (std::getline(*sub_iss, row_token, '/'))
            {
                row_tokens.push_back(row_token);
            }

            for (std::string row_token : row_tokens)
            {
                coli = 0;
                for (char c : row_token)
                {
                    if (std::isdigit(c))
                    {
                        coli += (int)c - (int)'0';
                        continue;
                    }

                    if( std::isupper(c) )
                    {
                        SET_BIT(_occ_bitboards[0], (7 - row_tokeni) * 8 + (7-coli));
                    }
                    else
                    {
                        SET_BIT(_occ_bitboards[1], (7 - row_tokeni) * 8 + (7-coli));
                    }

                    SET_BIT(_occ_bitboards[2], (7 - row_tokeni) * 8 + (7-coli));
                    SET_BIT(_piece_bitboards[charToPiece(c)], (7 - row_tokeni) * 8 + (7-coli));
                    coli++;
                }
                row_tokeni++;
            }

            delete sub_iss;
            break;

        case 1:
            /* side to move indicator */
            if (token == "w")
            {
                _side_to_move = Side::white;
            }
            else if (token == "b")
            {
                _side_to_move = Side::black;
            }
            else
            {
                _side_to_move = Side::both;
            }
            break;

        case 2:
            /* castling rights readout */
            for (char c : token)
            {
                switch (c)
                {
                case 'K':
                    SET_BIT(_castle_rights, 3);
                    break;
                case 'Q':
                    SET_BIT(_castle_rights, 2);
                    break;
                case 'k':
                    SET_BIT(_castle_rights, 1);
                    break;
                case 'q':
                    SET_BIT(_castle_rights, 0);
                    break;

                default:
                    break;
                }
            }
            break;

        case 3:
            /* en passant */
            if(token == "-")
            {
                _en_passant = NO_SQ;
                break;
            }
            _en_passant = Board::squareToInt(token[0], token[1]);
            break;

        case 4:
            /* half move clock */
            _halfmove_clock = std::stoi(token);
            break;

        case 5:
            /* full move clock */
            _fullmove_clock = std::stoi(token);
            break;

        default:
            break;
        }

        tokeni++;
    }
}

Board &Board::operator=(const Board &other)
{
    _side_to_move = other._side_to_move;
    _castle_rights = other._castle_rights;
    _en_passant = other._en_passant;
    _halfmove_clock = other._halfmove_clock;
    _fullmove_clock = other._fullmove_clock;

    std::copy(other._piece_bitboards, other._piece_bitboards + 12, _piece_bitboards);
    std::copy(other._occ_bitboards, other._occ_bitboards + 3, _occ_bitboards);
    
    return *this;
}

Piece Board::charToPiece(char pieceChar)
{
    switch (pieceChar)
    {
    case 'K':
        return K;
    case 'Q':
        return Q;
    case 'B':
        return B;
    case 'N':
        return N;
    case 'R':
        return R;
    case 'P':
        return P;
    case 'k':
        return k;
    case 'q':
        return q;
    case 'b':
        return b;
    case 'n':
        return n;
    case 'r':
        return r;
    case 'p':
        return p;
    default:
        std::cerr << "Invalid piece character: " << pieceChar << std::endl;
        return no_piece;
    }
}

char Board::PieceToChar(Piece piece)
{
    switch (piece)
    {
    case K:
        return 'K';
    case Q:
        return 'Q';
    case B:
        return 'B';
    case N:
        return 'N';
    case R:
        return 'R';
    case P:
        return 'P';
    case k:
        return 'k';
    case q:
        return 'q';
    case b:
        return 'b';
    case n:
        return 'n';
    case r:
        return 'r';
    case p:
        return 'p';
    default:
        return 'X';
    }
}

wchar_t Board::PieceToWChar(Piece piece)
{
    switch (piece)
    {
    case K:
        return L'♔';
    case Q:
        return L'♕';
    case B:
        return L'♗';
    case N:
        return L'♘';
    case R:
        return L'♖';
    case P:
        return L'♙';
    case k:
        return L'♚';
    case q:
        return L'♛';
    case b:
        return L'♝';
    case n:
        return L'♞';
    case r:
        return L'♜';
    case p:
        return L'♟';
    default:
        return L'X';
    }
}

U8 Board::squareToInt(char field, char rank)
{
    U8 ret = 0;

    switch (field)
    {
    case 'a':
        ret += _A;
        break;
    case 'b':
        ret += _B;
        break;
    case 'c':
        ret += _C;
        break;
    case 'd':
        ret += _D;
        break;
    case 'e':
        ret += _E;
        break;
    case 'f':
        ret += _F;
        break;
    case 'g':
        ret += _G;
        break;
    case 'h':
        ret += _H;
        break;
    default:
        break;
    }

    switch (rank)
    {
    case '1':
        ret += 8 * _1;
        break;
    case '2':
        ret += 8 * _2;
        break;
    case '3':
        ret += 8 * _3;
        break;
    case '4':
        ret += 8 * _4;
        break;
    case '5':
        ret += 8 * _5;
        break;
    case '6':
        ret += 8 * _6;
        break;
    case '7':
        ret += 8 * _7;
        break;
    case '8':
        ret += 8 * _8;
        break;
    default:
        break;
    }

    return ret;
}

char Board::intToField(U8 square)
{

    if(square == NO_SQ)
    {
        return '-';
    }

    switch (square % 8)
    {
    case _A:
        return 'a';
    case _B:
        return 'b';
    case _C:
        return 'c';
    case _D:
        return 'd';
    case _E:
        return 'e';
    case _F:
        return 'f';
    case _G:
        return 'g';
    case _H:
        return 'h';
    default:
        return '-';
    }
}

char Board::intToRank(U8 square)
{

    if(square == NO_SQ)
    {
        return '-';
    }

    switch (square / 8)
    {
    case _1:
        return '1';
    case _2:
        return '2';
    case _3:
        return '3';
    case _4:
        return '4';
    case _5:
        return '5';
    case _6:
        return '6';
    case _7:
        return '7';
    case _8:
        return '8';
    default:
        return '-';
    }
}

std::string Board::toString() const
{
    std::string ret;

    for (int sq = 63; sq > -1; sq--)
    {

        if( sq % 8 == 7 )
        {
            ret += '\n';
        }

        if(!GET_BIT(_occ_bitboards[2], sq))
        {
            ret += " .";
            continue;
        }

        for (size_t bitboard_index = 0; bitboard_index < 12; bitboard_index++)
        {
            if(GET_BIT(_piece_bitboards[bitboard_index], sq))
            {
                ret += ' ';
                ret += Board::PieceToChar(static_cast<Piece>(bitboard_index));
            }
        }
    }
    ret += '\n';
    ret += (_side_to_move == Side::white) ? "Side to move: White\n" : "Side to move: Black\n";
    
    ret += "Castle rights: ";

    if(GET_BIT(_castle_rights, 3))
    {
        ret += 'K';
    }
    if(GET_BIT(_castle_rights, 2))
    {
        ret += 'Q';
    }
    if(GET_BIT(_castle_rights, 1))
    {
        ret += 'k';
    }
    if(GET_BIT(_castle_rights, 0))
    {
        ret += 'q';
    }
    
    ret += "\nEn passant: ";
    ret += Board::intToField(_en_passant);
    ret += Board::intToRank(_en_passant);
    ret += "\nClocks: " + std::to_string(_halfmove_clock) + "/" + std::to_string(_fullmove_clock) + "\n";
    return ret;
}

std::wstring Board::toWString() const
{
    std::wstring ret;

    for (int sq = 63; sq > -1; sq--)
    {

        if( sq % 8 == 7 )
        {
            ret += L'\n';
        }

        if(!GET_BIT(_occ_bitboards[2], sq))
        {
            ret += L" ⎕";
            continue;
        }

        for (size_t bitboard_index = 0; bitboard_index < 12; bitboard_index++)
        {
            if(GET_BIT(_piece_bitboards[bitboard_index], sq))
            {
                ret += L' ';
                ret += Board::PieceToWChar(static_cast<Piece>(bitboard_index));
            }
        }
    }
    ret += L'\n';
    ret += (_side_to_move == Side::white) ? L"Side to move: White\n" : L"Side to move: Black\n";
    ret += L"Castle rights: ";

    if(GET_BIT(_castle_rights, 3))
    {
        ret += L'K';
    }
    if(GET_BIT(_castle_rights, 2))
    {
        ret += L'Q';
    }
    if(GET_BIT(_castle_rights, 1))
    {
        ret += L'k';
    }
    if(GET_BIT(_castle_rights, 0))
    {
        ret += L'q';
    }

    ret += L"\nEn passant: ";
    ret += Board::intToField(_en_passant);
    ret += Board::intToRank(_en_passant);
    ret += L"\nClocks: " + std::to_wstring(_halfmove_clock) + L"/" + std::to_wstring(_fullmove_clock) + L'\n';
    return ret;
}

FEN_t Board::getFEN()
{
    std::string ret;
    int zero_count = 0, sq;
    
    for(int i=7; i > -1; i--)
    {
        for(int j=7; j > -1; j--)
        {
            sq = i*8 + j;
            if( !GET_BIT(_occ_bitboards[2], sq) )
            {
                zero_count++;
                continue;
            }

            if( zero_count != 0 )
            {
                ret += std::to_string(zero_count);
                zero_count = 0;
            }

            for(int piecei = 0; piecei < 12; piecei++)
            {
                if(GET_BIT(_piece_bitboards[piecei], sq))
                {
                    ret += Board::PieceToChar(static_cast<Piece>(piecei));
                }
            }
        }

        if( zero_count != 0 )
        {
            ret += std::to_string(zero_count);
            zero_count = 0;
        }

        ret += "/";
    }

    if(_side_to_move == Side::white)
    {
        ret += " w ";
    }
    else
    {
        ret += " b ";
    }

    if(GET_BIT(_castle_rights, 3))
    {
        ret += "K";
    }
    if(GET_BIT(_castle_rights, 2))
    {
        ret += "Q";
    }
    if(GET_BIT(_castle_rights, 1))
    {
        ret += "k";
    }
    if(GET_BIT(_castle_rights, 0))
    {
        ret += "q";
    }

    ret += " ";

    if(_en_passant != NO_SQ)
    {
        ret += Board::intToField(_en_passant);
        ret += Board::intToRank(_en_passant);
    }
    else
    {
        ret += "-";
    }

    ret += " " + std::to_string(_halfmove_clock) + " " + std::to_string(_fullmove_clock);

    return ret;
}

std::string Board::moveToString(Move_t move)
{
    std::string ret;
    ret += Board::intToField(getSourceSquare(move));
    ret += Board::intToRank(getSourceSquare(move));
    ret += "/";
    ret += Board::intToField(getTargetSquare(move));
    ret += Board::intToRank(getTargetSquare(move));
    ret += " ";
    ret += Board::PieceToChar(getPromotionPiece(move));
    if(getCaptureFlag(move))
    {
        ret += " capture";
    }
    if(getDoublePawnPushFlag(move))
    {
        ret += " dpp";
    }
    if(getEnPassantFlag(move))
    {
        ret += " enpassant";
    }
    if(getCastleFlag(move))
    {
        ret += " castle";
    }
    if(getRookFlag(move))
    {
        ret += " rookmove";
    }
    return ret;
}

std::string Board::moveToStringShort(Move_t move)
{
    std::string ret;
    ret += Board::intToField(getSourceSquare(move));
    ret += Board::intToRank(getSourceSquare(move));
    ret += "/";
    ret += Board::intToField(getTargetSquare(move));
    ret += Board::intToRank(getTargetSquare(move));
    return ret;
}

bool Board::validFEN(FEN_t fen)
{
    return std::regex_match(fen, std::basic_regex(FEN_REGEX));
}
