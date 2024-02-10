#pragma once

#include "../utils/common/includes.h"
#include "game.h"

enum InterfaceType
{
    UCI,
    PERUN,

    NOTEXISTING
};

// main running function
void run();
InterfaceType getInterfaceType(const std::string command);
void message(const std::string message);
void warning(const std::string message);

/* ------------------------------- INTERFACES ------------------------------- */

class Interface
{
public:
    virtual void init();
    virtual void make_command(std::string command);
    virtual void end();

    Interface() = default;
    static void changeInterface(std::unique_ptr<Interface> &current, std::string change_command);
};

/* ----------------------------------- UCI ---------------------------------- */

enum UciOptionType
{
    CHECK,
    SPIN,
    COMBO,
    BUTTON,
    STRING
};

class UciInterface : public Interface
{
private:

    static std::thread _messenger_thread;
    static void messenger(EngineResults &engr, std::mutex &endmtx);
    static std::vector<std::string> no_args;
    static std::set<std::string> go_subcommands_set;

    // gui to engine
    static void uci(std::vector<std::string> &args);
    static void debug(std::vector<std::string> &args);
    static void isready(std::vector<std::string> &args);
    static void setoption(std::vector<std::string> &args);
    static void ucinewgame(std::vector<std::string> &args);
    static void position(std::vector<std::string> &args);
    static void go(std::vector<std::string> &args);
    static void stop(std::vector<std::string> &args);
    static void ponderhit(std::vector<std::string> &args);
    static void quit(std::vector<std::string> &args);

    static std::map<std::string, std::function<void(std::vector<std::string> &)>> commands_map;

    // engine to gui
    static void uciok();
    static void id(std::string engine_name, std::string engine_author);
    static void redyok();
    static void bestmove();
    static void info();
    static void option(std::string option_name, UciOptionType type,
        std::string default_val = "", std::string min = "", std::string max = "", std::string var = "");


    // misc
    static Move_t parse_move(std::string raw_move);

public:
    void init() override; // used in constructor (declared outside for manual reset without creating new object)
    void make_command(std::string command) override;
    void end() override; // used in destructor

    UciInterface();
    ~UciInterface();

    static bool debug_mode;
};

/* ---------------------------------- PERUN --------------------------------- */

class PerunInterface : public Interface
{
public:
    void init() override; // used in constructor
    void make_command(std::string command) override;
    void end() override; // used in destructor

    PerunInterface();
    ~PerunInterface();
};

/* ---------------------------------- MICS ---------------------------------- */

std::string str_tolower(std::string &s);
std::string str_tolower_noref(const std::string s);
std::string clean_newlines(const std::string s);
