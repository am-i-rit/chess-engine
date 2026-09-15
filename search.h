#pragma once
#include <cstdint>
#include "chessboard.h"

struct SearchResult
{
    Move move;
    int score;
    bool foundMove;
};

class Search
{
private:

    static const int middlegameTables[6][64];
    static const int endgameTables[6][64];
    
    int scoreMove(const Chessboard& board, const Move &move) const;
    void orderMoves(const Chessboard &board, Move* moves, int numMoves) const;

    std::uint64_t nodes;
    int evaluate(const Chessboard& board) const;    
    int negamax(Chessboard& board, int depth, int alpha, int beta);

    int quiesce(Chessboard &board, int alpha, int beta);

    void moveToFront(Move* moves, int numMoves, const Move& preferredMove) const;

    SearchResult searchRoot(Chessboard& board, int depth, const Move& preferredMove);



public:

    Move findBestMove(Chessboard& board, int depth);
    std::uint64_t getNodes() const;

};