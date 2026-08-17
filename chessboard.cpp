#include "chessboard.h"

Chessboard::Chessboard()
{
    bitboards[wPawn]    = 0x00ff000000000000;
    bitboards[wKnight]  = 0x4200000000000000;
    bitboards[wBishop]  = 0x2400000000000000;
    bitboards[wRook]    = 0x8100000000000000;
    bitboards[wQueen]   = 0x0800000000000000;
    bitboards[wKing]    = 0x1000000000000000;
    bitboards[bPawn]    = 0x000000000000ff00;
    bitboards[bKnight]  = 0x0000000000000042;
    bitboards[bBishop]  = 0x0000000000000024;
    bitboards[bRook]    = 0x0000000000000081;
    bitboards[bQueen]   = 0x0000000000000008;
    bitboards[bKing]    = 0x0000000000000010;

    // fill mailbox to match the bitboards above
    for (int sq = 0; sq < 64; ++sq)
        mailbox[sq] = Empty;

    for (int piece = 0; piece < 12; ++piece)
        for (int sq = 0; sq < 64; ++sq)
            if (bitboards[piece] & (uint64_t(1) << sq))
                mailbox[sq] = piece;    
}

uint8_t Chessboard::getPiece(uint8_t square)
{
    return mailbox[square];
}

void Chessboard::setPiece(uint8_t piece, uint8_t square)
{
    uint64_t mask = uint64_t(1) << square;

    // clear this square from whichever bitboard currently owns it
    uint8_t current = mailbox[square];
    if (current != Empty)
        bitboards[current] &= ~mask;

    // set the new piece, if any
    if (piece != Empty)
        bitboards[piece] |= mask;

    mailbox[square] = piece;
}