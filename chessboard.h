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
    WHITE = 13, BLACK, DRAWN
};

// file masks
static uint64_t FILE_A = 0x0101010101010101ULL;
static uint64_t FILE_B = 0x0202020202020202ULL;
static uint64_t FILE_G = 0x4040404040404040ULL;
static uint64_t FILE_H = 0x8080808080808080ULL;

// rank masks for double pawn pushes
static uint64_t RANK_3 = 0x0000FF0000000000ULL;
static uint64_t RANK_6 = 0x0000000000FF0000ULL;


struct Move
{
    // coordinates for move
    std::uint8_t from;
    std::uint8_t to;

    // specify promotions
    std::uint8_t promotion;

    // overload operator
    bool operator== (const Move& move) const
    {
        return from == move.from && to == move.to && promotion == move.promotion;
    }
};

struct boardState
{
    // create a bitboard for each piece type for each colour
    std::uint64_t bitboards[12];

    // variables for whether castling rights exist
    bool wKingside, bKingside, wQueenside, bQueenside;

    // bitboard to represent en passant target square
    std::uint64_t enpTarget;

    // tracks turn
    Colour turn;

    // tracks number of half moves (for 50 move rule)
    std::uint8_t numHalfMoves;

    // hash (helps w threefold repitition)
    std::uint64_t zobristHash;
};

class Chessboard
{
 private:

    boardState* stateStack;
    int stackIndex;

    // attack bitboards
    uint64_t kingAttacks[64];
    uint64_t knightAttacks[64];
    uint64_t diagonalRays[64][4];
    uint64_t orthogonalRays[64][4];


 public:
    std::uint8_t getPiece(std::uint8_t square);
    void setPiece(std::uint8_t piece, std::uint8_t square);

    // make moves
    void move(const Move &move);
    void undo();

    Colour getTurn();

    // methods for move gen
    void pseudoMoves(Move* moves, int& numMoves);
    bool isLegal(const Move& move);
    bool isAttacked(uint8_t square, uint8_t colour);
    uint8_t wKingSquare();
    uint8_t bKingSquare();

    // check for game ending
    std::uint8_t gameResult();
    bool isDrawn();

    // constructor
    Chessboard(); 

    // destructor
    ~Chessboard();
};