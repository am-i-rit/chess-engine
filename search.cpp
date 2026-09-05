// search.cpp
#include "search.h"
#include "helper.h"

int Search::evaluate(const Chessboard &board) const
{
    // returns an evaluation score relative to white
    // the sign of the score will be made correct in negamax function
    static constexpr int pieceValues[12] = {
        100, 300, 320, 500, 900, 0, 
        -100, -300, -320, -500, -900, 0
    }; 

    int evaluation = 0;

    for(int i = 0; i < 12; ++i)
    {
        evaluation += popcount(board.getBitboard(i)) * pieceValues[i];
    }
    
    return evaluation;
}

int Search::negamax(Chessboard& board, int depth)
{
    ++nodes;

    // if (board.isDrawn())
    // {
    //     return 0;
    // }

    // Depth limit
    if (depth <= 0)
    {
        uint8_t result = board.gameResult();

        if (result == WHITE || result == BLACK)
        {
            return -100000;
        }

        if (result == DRAWN)
        {
            return 0;
        }

        // evaluate() is positive for White and negative for Black.
        if (board.getTurn() == WHITE)
        {
            return evaluate(board);
        }
        else
        {
            return -evaluate(board);
        }
    }

    Move moves[256];
    int numMoves;
    board.pseudoMoves(moves, numMoves);

    int bestScore = -1000000;
    int legalMoves = 0;

    for (int i = 0; i < numMoves; ++i)
    {
        board.move(moves[i]);

        bool illegal;

        if (board.getTurn() == WHITE)
        {
            illegal = board.isAttacked(
                board.bKingSquare(),
                WHITE
            );
        }
        else
        {
            illegal = board.isAttacked(
                board.wKingSquare(),
                BLACK
            );
        }

        if (!illegal)
        {
            ++legalMoves;

            int score = -negamax(board, depth - 1);

            if (score > bestScore)
            {
                bestScore = score;
            }
        }

        board.undo();
    }

    if (legalMoves == 0)
    {
        bool inCheck;

        if (board.getTurn() == WHITE)
        {
            inCheck = board.isAttacked(
                board.wKingSquare(),
                BLACK
            );
        }
        else
        {
            inCheck = board.isAttacked(
                board.bKingSquare(),
                WHITE
            );
        }

        if (inCheck)
        {
            return -100000 - depth;
        }

        return 0; // stalemate
    }

    return bestScore;
}

Move Search::findBestMove(Chessboard& board, int depth)
{
    nodes = 0;

    Move moves[256];
    int numMoves;
    board.pseudoMoves(moves, numMoves);

    Move bestMove = {64, 64, EMPTY};
    int bestScore = -1000000;
    bool foundMove = false;

    for (int i = 0; i < numMoves; ++i)
    {
        board.move(moves[i]);

        bool illegal;

        if (board.getTurn() == WHITE)
        {
            illegal = board.isAttacked(
                board.bKingSquare(),
                WHITE
            );
        }
        else
        {
            illegal = board.isAttacked(
                board.wKingSquare(),
                BLACK
            );
        }

        if (!illegal)
        {
            int score = -negamax(board, depth - 1);

            if (!foundMove || score > bestScore)
            {
                bestScore = score;
                bestMove = moves[i];
                foundMove = true;
            }
        }

        board.undo();
    }

    return bestMove;
}

uint64_t Search::getNodes() const
{
    return nodes;
}

