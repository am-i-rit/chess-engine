#include "chessboard.h"
#include <iostream>
#include "perft.h"

std::uint64_t perft(Chessboard& board, int depth)
{
    Move moves[256];
    int numMoves;

    if (depth == 0)
    {
        return 1;
    }

    board.pseudoMoves(moves, numMoves);

    std::uint64_t nodes = 0;

    for (int i = 0; i < numMoves; ++i)
    {
        board.move(moves[i]);

        bool illegal;

        if (board.getTurn() == (WHITE)) 
        // if turn is white then black just moved as i switch turns in my move function
        {
            illegal = board.isAttacked(board.bKingSquare(), WHITE);
        }
        else
        {
            illegal = board.isAttacked(board.wKingSquare(), BLACK);
        } 
        
        if (!illegal)
        {
            nodes += perft(board, (depth - 1));
        }

        board.undo();
    }

    return nodes;
}  

void perftDivide(Chessboard& board, int depth)
{
    Move moves[256];
    int numMoves;

    board.pseudoMoves(moves, numMoves);

    std::uint64_t total = 0;

    for (int i = 0; i < numMoves; ++i)
    {
        if (!board.isLegal(moves[i]))
        {
            continue;
        }

        board.move(moves[i]);
        std::uint64_t nodes = perft(board, depth - 1);
        board.undo();

        char fromFile = 'a' + moves[i].from % 8;
        char fromRank = '8' - moves[i].from / 8;
        char toFile = 'a' + moves[i].to % 8;
        char toRank = '8' - moves[i].to / 8;

        std::cout << fromFile << fromRank
                  << toFile << toRank
                  << ": " << nodes << '\n';

        total += nodes;
    }

    std::cout << "Total: " << total << '\n';
}