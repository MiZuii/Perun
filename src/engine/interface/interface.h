#pragma once

#include "../utils/common/includes.h"

// main running function
void run();
bool is_interface(std::string command);

/* ------------------------------- INTERFACES ------------------------------- */

class Interface
{
protected:
    virtual void init();
    virtual void make_command(std::string command);
    virtual void end();

public:
    Interface() = default;
};


class UciInterface : public Interface
{
public:
    void init() override;
    void make_command(std::string command) override;
    void end() override;
};


class PerunInterface : public Interface
{
public:
    void init() override;
    void make_command(std::string command) override;
    void end() override;
};
