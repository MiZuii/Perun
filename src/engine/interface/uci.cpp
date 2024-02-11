#include "interface.h"

std::thread UciInterface::_messenger_thread;
bool UciInterface::debug_mode = false;
std::map<std::string, std::function<void(std::vector<std::string> &)>> UciInterface::commands_map = {
    {"uci", [](std::vector<std::string> &args) -> void { uci(args); }},
    {"isready", [](std::vector<std::string> &args) -> void { isready(args); }},
    {"debug", [](std::vector<std::string> &args) -> void { debug(args); }},
    {"setoption", [](std::vector<std::string> &args) -> void { setoption(args); }},
    {"ucinewgame", [](std::vector<std::string> &args) -> void { ucinewgame(args); }},
    {"position", [](std::vector<std::string> &args) -> void { position(args); }},
    {"go", [](std::vector<std::string> &args) -> void { go(args); }},
    {"stop", [](std::vector<std::string> &args) -> void { stop(args); }},
    {"ponderhit", [](std::vector<std::string> &args) -> void { ponderhit(args); }},
    {"quit", [](std::vector<std::string> &args) -> void { quit(args); }}
};
std::set<std::string> UciInterface::go_subcommands_set = {
    "searchmoves", 
    "ponder", 
    "wtime", 
    "btime", 
    "winc", 
    "binc", 
    "movestogo", 
    "depth", 
    "nodes", 
    "mate", 
    "movetime", 
    "infinite"
};
std::vector<std::string> UciInterface::no_args;

std::vector<std::string> tokenize(std::string command)
{
    auto const re = std::regex{R"(\s+)"};
    auto const vec = std::vector<std::string>(
        std::sregex_token_iterator{command.begin(), command.end(), re, -1},
        std::sregex_token_iterator{}
    );
    return vec;
}

/* ------------------------- COMMANDS IMPLEMENTATION ------------------------ */

void UciInterface::uci(std::vector<std::string> &args)
{
    id("Perun", "Piotr Wiercigroch");
    // place for engine options (none for now)
    uciok();
}

void UciInterface::isready(std::vector<std::string> &args)
{
    redyok();
}

void UciInterface::debug(std::vector<std::string> &args)
{
    if(!args[1].compare("on"))
    {
        debug_mode = true;
    }
    else if(!args[1].compare("off"))
    {
        debug_mode = false;
    }
    else
    {
        warning("Invalid command arguments");
    }
}

void UciInterface::setoption(std::vector<std::string> &args)
{
    // no options for now
}

void UciInterface::ucinewgame(std::vector<std::string> &args)
{
    if(Game::game_running)
    {
        warning("Cannot use this command right now, Engine is searching");
        return;
    }

    Game::clear();
    Game::create();
}

void UciInterface::position(std::vector<std::string>& args)
{
    // prepare game obj
    if(Game::game_running)
    {
        warning("Cannot use this command right now, Engine is searching");
        return;
    }

    if(!Game::game_initialized)
    {
        Game::create();
    }

    // position parsing
    FEN_t fen;
    bool custom_fen = false;

    if(!args[1].compare("startpos"))
    {
        Game::setPosition(STARTING_POS);
    }
    else if(!args[1].compare("fen"))
    {
        //parse fen
        custom_fen = true;
        if(args.size() < 3)
        {
            warning("Invalid command arguments");
            return;
        }

        std::vector<std::string> fen_vector(args.begin() + 2, std::find(args.begin(), args.end(), "moves"));
        fen = std::accumulate(fen_vector.begin(), fen_vector.end(), std::string{},
                [](const std::string& acc, const std::string& item) {
                  return acc.empty() ? item : acc + " " + item;
                });
        Game::setPosition(fen);
    }
    else
    {
        warning("Invalid command arguments");
        return;
    }

    // moves parsing
    if(std::find(args.begin(), args.end(), "moves") != args.end())
    {
        std::vector<Move_t> converted_moves;
        Board simulation_board(custom_fen ? fen : STARTING_POS);

        for(auto it = std::find(args.begin(), args.end(), "moves") + 1; it != args.end(); ++it)
        {
            converted_moves.push_back(simulation_board.createAmbiguousMove(parse_move(*it)));
            simulation_board.makeMove(converted_moves.back());
        }

        Game::setMoves(converted_moves);
    }
}

void UciInterface::go(std::vector<std::string> &args)
{
    assert(Game::game_initialized);
    assert(!Game::game_running);

    SearchArgs args_fill;

    // parse go arguments
    if(std::find(args.begin(), args.end(), "infinity") != args.end())
    {
        args_fill.search_type = DEPTH_LIM;
        args_fill.depth_lim = SEARCH_INF;
    }

    if(std::find(args.begin(), args.end(), "depth") != args.end())
    {
        args_fill.search_type = DEPTH_LIM;
        args_fill.depth_lim = std::stoi(*++std::find(args.begin(), args.end(), "depth"));
    }

    if(std::find(args.begin(), args.end(), "movetime") != args.end())
    {
        args_fill.search_type = TIME_LIM;
        args_fill.depth_lim = std::stoi(*++std::find(args.begin(), args.end(), "movetime"));
    }

    if(std::find(args.begin(), args.end(), "nodes") != args.end())
    {
        args_fill.search_type = NODE_LIM;
        args_fill.depth_lim = std::stoi(*++std::find(args.begin(), args.end(), "nodes"));
    }

    Game::setSearchArgs(args_fill);

    // initialize messenger thread
    if(_messenger_thread.joinable())
    {
        _messenger_thread.join();
    }
    _messenger_thread = std::thread(UciInterface::messenger, std::ref(Game::getEngineResultsStruct()), std::ref(Game::getEngineResultsMutex()));

    // start search
    Game::start();
}

void UciInterface::stop(std::vector<std::string> &args)
{
    Game::stop();
}

void UciInterface::ponderhit(std::vector<std::string> &args)
{
}

void UciInterface::quit(std::vector<std::string> &args)
{
    Game::destroy();
    _messenger_thread.join();
    exit(0); //??
}

void UciInterface::uciok()
{
    message("uciok");
}

void UciInterface::id(std::string engine_name, std::string engine_author)
{
    message("id name " + engine_name);
    message("id author " + engine_author);
}

void UciInterface::redyok()
{
    message("redyok");
}

void UciInterface::bestmove()
{

}

void UciInterface::info()
{

}

void UciInterface::option(std::string option_name, UciOptionType type,
    std::string default_val, std::string min, std::string max, std::string var)
{

}

void UciInterface::messenger(EngineResults &engr, std::mutex &endmtx)
{
    while(true)
    {
        endmtx.lock();

        if(!engr.new_data)
        {
            endmtx.unlock();
            continue;
        }

        // new data -> perform sending
        message("bestmove: " + unparse_move(engr.best_move));

        // mark that data was read
        engr.new_data = false;

        if(engr.finished)
        {
            endmtx.unlock();
            break;
        }

        endmtx.unlock();
    }
}

/* ----------------------------- COMMANDS RUNNER ---------------------------- */

void UciInterface::make_command(std::string command)
{
    std::vector<std::string> tokens = tokenize(command);

    if(commands_map.find(tokens[0]) == commands_map.end())
    {
        warning("unknown command");
        return;
    }

    commands_map[tokens[0]](tokens);
}

Move_t UciInterface::parse_move(std::string raw_move)
{
    if(raw_move.length() < 4)
    {
        return createMove(NO_SQ, NO_SQ, no_piece, no_piece, false, false, false, false, false, false);
    }

    U8 source_square = Board::squareToInt(raw_move[0], raw_move[1]);
    U8 target_square = Board::squareToInt(raw_move[2], raw_move[3]);

    if(raw_move.length() == 4)
    {
        return createMove(source_square, target_square, no_piece, no_piece, false, false, false, false, false, false);
    }

    Piece promotion_piece = Board::charToPiece(raw_move[4]);
    return createMove(source_square, target_square, no_piece, promotion_piece, false, false, false, false, false, false);
}

std::string UciInterface::unparse_move(Move_t move)
{
    std::string res;
    res += Board::intToField(getSourceSquare(move));
    res += Board::intToRank(getSourceSquare(move));
    res += Board::intToField(getTargetSquare(move));
    res += Board::intToRank(getTargetSquare(move));
    return res;
}
