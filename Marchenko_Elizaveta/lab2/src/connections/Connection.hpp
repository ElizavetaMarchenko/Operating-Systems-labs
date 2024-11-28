#pragma once
#include <iostream>

class Connection
{
public:
    virtual bool Read(std::string&) = 0;
    virtual bool Write(const std::string&) = 0;
    virtual ~Connection() = default;

    Connection() = default;
};

