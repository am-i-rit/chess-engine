#include "chessboard.h"

Chessboard::Chessboard()
{
    stateStack = new boardState[1000];
    stackIndex = 0;

    stateStack[0].bitboards[wPawn]    = 0x00ff000000000000;
    stateStack[0].bitboards[wKnight]  = 0x4200000000000000;
    stateStack[0].bitboards[wBishop]  = 0x2400000000000000;
    stateStack[0].bitboards[wRook]    = 0x8100000000000000;
    stateStack[0].bitboards[wQueen]   = 0x0800000000000000;
    stateStack[0].bitboards[wKing]    = 0x1000000000000000;
    stateStack[0].bitboards[bPawn]    = 0x000000000000ff00;
    stateStack[0].bitboards[bKnight]  = 0x0000000000000042;
    stateStack[0].bitboards[bBishop]  = 0x0000000000000024;
    stateStack[0].bitboards[bRook]    = 0x0000000000000081;
    stateStack[0].bitboards[bQueen]   = 0x0000000000000008;
    stateStack[0].bitboards[bKing]    = 0x0000000000000010;

    stateStack[0].kingsideW = true;
    stateStack[0].queensideW = true;
    stateStack[0].kingsideB = true;
    stateStack[0].queensideB = true;

    stateStack[0].enpTarget = 0;

    stateStack[0].turn = WHITE;
}

uint8_t Chessboard::getPiece(uint8_t square)
{
    uint64_t mask = uint64_t(1) << square;
    for (int i = 0; i < 12; ++i)
        if (stateStack[stackIndex].bitboards[i] & mask) return i;
    return EMPTY;
}

void Chessboard::setPiece(uint8_t piece, uint8_t square)
{
    uint64_t mask = uint64_t(1) << square;

    uint8_t current = getPiece(square);   // now a scan instead of an array read
    if (current != EMPTY)
        stateStack[stackIndex].bitboards[current] &= ~mask;

    if (piece != EMPTY)
        stateStack[stackIndex].bitboards[piece] |= mask;
}

void Chessboard::move(const Move& move)
{
    int newIndex = stackIndex + 1;
    stateStack[newIndex] = stateStack[stackIndex];
    stackIndex = newIndex;

    uint64_t fromBitboard = uint64_t(1) << move.from;
    uint64_t toBitboard   = uint64_t(1) << move.to;
    uint64_t moveBitboard = fromBitboard | toBitboard;

    bool whiteToMove = (stateStack[stackIndex].turn == WHITE);

    // move the piece
    int start = whiteToMove ? 0 : 6;
    int end   = whiteToMove ? 6 : 12;
    for (int i = start; i < end; ++i)
    {
        if (stateStack[stackIndex].bitboards[i] & fromBitboard)
        {
            stateStack[stackIndex].bitboards[i] ^= moveBitboard;
            break;
        }
    }

    // capture, if any
    int capStart = whiteToMove ? 6 : 0;
    int capEnd   = whiteToMove ? 12 : 6;
    for (int i = capStart; i < capEnd; ++i)
    {
        if (stateStack[stackIndex].bitboards[i] & toBitboard)
        {
            stateStack[stackIndex].bitboards[i] ^= toBitboard;
            break;
        }
    }

    stateStack[stackIndex].turn = whiteToMove ? BLACK : WHITE;
}

void Chessboard::undo()
{
    if (stackIndex > 0) --stackIndex;
}

Colour Chessboard::getTurn()
{
    return stateStack[stackIndex].turn;
}

Chessboard::~Chessboard()
{
    delete[] stateStack;    
}