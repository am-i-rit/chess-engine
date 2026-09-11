// search.cpp
#include "search.h"
#include "helper.h"


const int Search::middlegameTables[6][64] = {
    // Pawn table
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    },

    // Knight table
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-20,-30,-30,-30,-30,-20,-50,
    },

    // Bishop table
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },

    // rook table
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    },

    // queen table
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },

    //king table
    {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    }
};

const int Search::endgameTables[6][64] = {
    // Same six-piece structure
};


int Search::evaluate(const Chessboard &board) const
{
    // returns an evaluation score relative to white
    // the sign of the score will be made correct in the negamax function
    static constexpr int pieceValues[12] = {
        100, 300, 320, 500, 900, 0, 
        -100, -300, -320, -500, -900, 0
    }; 

    int evaluation = 0;

    for(int i = 0; i < 12; ++i)
    {
        uint64_t bb = board.getBitboard(i);
        evaluation += popcount(bb) * pieceValues[i];

        while (bb) {
            int square = lsbIndex(bb);
            bb &= bb - 1;

            if (i < 6) {
                evaluation += middlegameTables[i][square];
            }
            else {
                int pieceType = i - 6;
                int mirroredSquare = square ^ 56;

                evaluation -= middlegameTables[pieceType][mirroredSquare];
            }
        }
    }   
    return evaluation;
}

int Search::negamax(Chessboard& board, int depth, int alpha, int beta)
{
    ++nodes;

    // if (board.isDrawn())
    // {
    //     return 0;
    // }

    // depth limit
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

            int score = -negamax(board, depth - 1, -beta, -alpha);

            if (score > alpha)
            {
                alpha = score;
            }
        }

        board.undo();

        if (alpha >= beta) break;
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

    return alpha;
}

Move Search::findBestMove(Chessboard& board, int depth)
{
    nodes = 0;

    Move moves[256];
    int numMoves;
    board.pseudoMoves(moves, numMoves);

    Move bestMove = {64, 64, EMPTY};
    int alpha = -1000000;
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
            int score = -negamax(board, depth - 1, -1000000, -alpha);

            if (!foundMove || score > alpha)
            {
                alpha = score;
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

