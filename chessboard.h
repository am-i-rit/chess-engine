#pragma once
#include <cstdint>

enum Piece: uint8_t
{
    wPawn, wKnight, wBishop, wRook, wQueen, wKing,
    bPawn, bKnight, bBishop, bRook, bQueen, bKing,
    Empty
};


class Chessboard
{
 private:
    std::uint64_t bitboards[12];
    std::uint8_t mailbox[64];


 public:
    std::uint8_t getPiece(std::uint8_t square);
    void setPiece(std::uint8_t piece, std::uint8_t square);


    Chessboard(); // constructor
};