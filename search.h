#pragma once
#include <cstdint>
#include "chessboard.h"


class Search
{
private:

    static const int middlegameTables[6][64];
    static const int endgameTables[6][64];

    std::uint64_t nodes;
    int evaluate(const Chessboard& board) const;    
    int negamax(Chessboard& board, int depth, int alpha, int beta);

public:

    Move findBestMove(Chessboard& board, int depth);
    std::uint64_t getNodes() const;

};