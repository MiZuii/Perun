#include "interface.h"

#include <algorithm>
#include <cctype>

using namespace std;

std::string str_tolower(std::string &s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

void run()
{
    cout << "Welcome to Perun!" << endl;

    // setup interface and deafult init
    Interface interface = PerunInterface();
    string command;

    while(true)
    {
        getline(cin, command);
        str_tolower(command);
    }
}

bool is_interface(std::string command)
{
    return false;
}

/* -------------------------------------------------------------------------- */
/*                                     UCI                                    */
/* -------------------------------------------------------------------------- */

void UciInterface::init()
{
}

void UciInterface::make_command(std::string command)
{
}

void UciInterface::end()
{
}

/* -------------------------------------------------------------------------- */
/*                                    PERUN                                   */
/* -------------------------------------------------------------------------- */

void PerunInterface::init()
{
}

void PerunInterface::make_command(std::string command)
{
}

void PerunInterface::end()
{
}

/* -------------------------------- INTERFACE ------------------------------- */

void Interface::init()
{
    throw std::runtime_error("Interface class methods are not meant for direct use.");
}

void Interface::make_command(std::string command)
{
    throw std::runtime_error("Interface class methods are not meant for direct use.");
}

void Interface::end()
{
    throw std::runtime_error("Interface class methods are not meant for direct use.");
}
