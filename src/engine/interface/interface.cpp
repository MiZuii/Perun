#include "interface.h"

void run()
{
    // init first instance of game
    Game::getInstance();

    std::cout << "Welcome to Perun!" << std::endl;

    // setup interface and deafult init
    std::unique_ptr<Interface> interface = std::make_unique<PerunInterface>();
    std::string command;

    while(true)
    {
        getline(std::cin, command);

        if( NOTEXISTING != getInterfaceType(command) )
        {
            Interface::changeInterface(interface, command);
            // NO continue because of uci interface
        }

        interface->make_command(command);
    }
}

InterfaceType getInterfaceType(const std::string command)
{
    std::string commandc = command;
    commandc.erase(std::remove_if(commandc.begin(), 
                                    commandc.end(),
                                [](unsigned char x) { return std::isspace(x); }),
               commandc.end());

    if( 0 == commandc.compare("uci") )
    {
        return UCI;
    }
    else if( 0 == commandc.compare("perun") )
    {
        return PERUN;
    }
    else
    {
        return NOTEXISTING;
    }
}

void message(const std::string message)
{
    std::cout << "> " << message << std::endl;
}

void warning(const std::string message)
{
    std::cerr << ">(warning) " << message << std::endl;
}

/* -------------------------------------------------------------------------- */
/*                                     UCI                                    */
/* -------------------------------------------------------------------------- */

void UciInterface::init()
{
}

void UciInterface::end()
{
}

UciInterface::UciInterface()
{
    init();
}

UciInterface::~UciInterface()
{
    end();
}

/* -------------------------------------------------------------------------- */
/*                                    PERUN                                   */
/* -------------------------------------------------------------------------- */

void PerunInterface::init()
{
}

void PerunInterface::end()
{
}

PerunInterface::PerunInterface()
{
    init();
}

PerunInterface::~PerunInterface()
{
    end();
}

/* -------------------------------- INTERFACE ------------------------------- */

void Interface::init()
{
    throw std::runtime_error("Interface class methods are not meant for direct use.");
}

void Interface::make_command(std::string command __attribute__((unused)))
{
    throw std::runtime_error("Interface class methods are not meant for direct use.");
}

void Interface::end()
{
    throw std::runtime_error("Interface class methods are not meant for direct use.");
}

void Interface::changeInterface(std::unique_ptr<Interface> &current, const std::string change_command)
{
    switch (getInterfaceType(change_command))
    {
    case UCI:
        current = std::make_unique<UciInterface>();
        break;

    case PERUN:
        current = std::make_unique<PerunInterface>();
        break;

    default:
        warning("Not implemented interface type. Starting Perun interface.");
        current = std::make_unique<PerunInterface>();
    }
}

std::string str_tolower(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string str_tolower_noref(const std::string sr)
{
    std::string s = sr;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string clean_newlines(const std::string sr)
{
    std::string s = sr;
    if(s.find('\n') < s.length())
    {
        s.erase('\n');
    }
    return s;
}
