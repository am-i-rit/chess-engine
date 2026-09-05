#pragma once
#include <cstdint>
#include "chessboard.h"


class Search
{
private:

    std::uint64_t nodes;
    
    int evaluate(const Chessboard& board) const;
    int negamax(Chessboard& board, int depth);

public:

    Move findBestMove(Chessboard& board, int depth);
    std::uint64_t getNodes() const;

    // constructor
    Search();
};