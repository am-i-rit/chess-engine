#pragma once

#include <string>

#include "chessboard.h"
#include "search.h"

class UCI
{
private:
    Chessboard board;
    Search search;

    Move parseMove(const std::string& text);
    std::string moveToString(const Move& move) const;

    void setPosition(const std::string& command);
    void go(const std::string& command);

public:
    void run();
};