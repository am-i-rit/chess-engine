#pragma once
#include <cstdint>

enum Piece: uint8_t
{
    wPawn, wKnight, wBishop, wRook, wQueen, wKing,
    bPawn, bKnight, bBishop, bRook, bQueen, bKing,
    EMPTY
};

enum Colour: uint8_t
{
    // using 13 and 14 because 0 through 12 are taken
    // by piece bitboards and idk if any overlap will occur
    WHITE = 13, BLACK
};

struct Move
{
    // coordinates for move
    std::uint8_t from;
    std::uint8_t to;

    // specify promotions
    std::uint8_t promotion;
};

struct boardState
{
    // bitboard for each piece type
    std::uint64_t bitboards[12];

    // boolean variables for castling rights
    bool wKingside, bKingside, wQueenside, bQueenside;

    // bitboard to represent en passant target
    std::uint64_t enpTarget;

    // tracks turn
    Colour turn;
 
};

class Chessboard
{
 private:

    boardState* stateStack;
    int stackIndex;



 public:
    std::uint8_t getPiece(std::uint8_t square);
    void setPiece(std::uint8_t piece, std::uint8_t square);

    // make moves
    void move(const Move &move);
    void undo();

    Colour getTurn();

    // constructor
    Chessboard(); 

    // destructor
    ~Chessboard();
};