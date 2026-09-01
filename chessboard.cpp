#include "chessboard.h"
#include "helper.h"
#include <cmath>

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

    stateStack[0].wKingside = true;
    stateStack[0].wQueenside = true;
    stateStack[0].bKingside = true;
    stateStack[0].bQueenside = true;

    stateStack[0].enpTarget = 0;

    stateStack[0].turn = WHITE;

    // generate various bitboards for piece attacks
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
        {
            uint8_t squareIndex = x + 8 * y;

            // king
            kingAttacks[squareIndex] = 0;

            int kdx[8] = { -1, -1, -1,  0, 0,  1, 1, 1 };
            int kdy[8] = { -1,  0,  1, -1, 1, -1, 0, 1 };

            for (int dir = 0; dir < 8; ++dir)
            {
                int nx = x + kdx[dir];
                int ny = y + kdy[dir];

                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8)
                {
                    uint8_t targetSquare = nx + 8 * ny;
                    kingAttacks[squareIndex] |= (uint64_t(1) << targetSquare);
                }
            }

            // knight
            knightAttacks[squareIndex] = 0;

            int ndx[8] = {1, -1, 1, -1, 2, -2, 2, -2};
            int ndy[8] = {2, 2, -2, -2, 1, 1, -1, -1};

            for (int dir = 0; dir < 8; ++dir)
            {
                int nx = x + ndx[dir];
                int ny = y + ndy[dir];

                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8)
                {
                    uint8_t targetSquare = nx + 8 * ny;
                    kingAttacks[squareIndex] |= (uint64_t(1) << targetSquare);
                }
            }


        }
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
    uint8_t movedPiece = EMPTY;
    int start = whiteToMove ? 0 : 6;
    int end   = whiteToMove ? 6 : 12;
    for (int i = start; i < end; ++i)
    {
        if (stateStack[stackIndex].bitboards[i] & fromBitboard)
        {
            stateStack[stackIndex].bitboards[i] ^= moveBitboard;
            movedPiece = i;
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

    // en passant
    if (toBitboard == stateStack[stackIndex].enpTarget)
    {
        if (movedPiece == wPawn)
        {
            stateStack[stackIndex].bitboards[bPawn] ^= stateStack[stackIndex].enpTarget << 8;
        }
        else if (movedPiece == bPawn)
        {
            stateStack[stackIndex].bitboards[wPawn] ^= stateStack[stackIndex].enpTarget >> 8;
        }
    }

    // update en passant target square
    if ((movedPiece == wPawn || movedPiece == bPawn) && abs(move.from - move.to) == 16)
    {
        uint8_t skippedSquare = (move.from + move.to) / 2;
        stateStack[stackIndex].enpTarget = uint64_t(1) << skippedSquare;
    }
    else 
    {
        stateStack[stackIndex].enpTarget = 0;
    }

    // castling
    if (movedPiece == wKing)
	{
		if (moveBitboard == 0x5000000000000000) 
        {
            stateStack[stackIndex].bitboards[wRook] ^= 0xa000000000000000;
        }
		else if (moveBitboard == 0x1400000000000000) 
        {
            stateStack[stackIndex].bitboards[wRook] ^= 0x0900000000000000;
        }
	}
	else if (movedPiece == bKing)
	{
		if (moveBitboard == 0x0000000000000050) 
        {
            stateStack[stackIndex].bitboards[bRook] ^= 0x00000000000000a0;
        }
		else if (moveBitboard == 0x0000000000000014) 
        {
            stateStack[stackIndex].bitboards[bRook] ^= 0x0000000000000009;
        }
	}

    // update castling rights
    if (moveBitboard & 0x9000000000000000) stateStack[stackIndex].wKingside = false;
	if (moveBitboard & 0x1100000000000000) stateStack[stackIndex].wQueenside = false;
	if (moveBitboard & 0x0000000000000090) stateStack[stackIndex].bKingside = false;
	if (moveBitboard & 0x0000000000000011) stateStack[stackIndex].bQueenside = false;

    // promotions
    if (move.promotion != EMPTY)
    {
        uint8_t pawnType;
        if (stateStack[stackIndex].turn == WHITE) pawnType = wPawn;
        else pawnType = bPawn;

        stateStack[stackIndex].bitboards[pawnType] ^= toBitboard;
        stateStack[stackIndex].bitboards[move.promotion] ^= toBitboard;
    }

    // switch turns
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

void Chessboard::pseudoMoves(Move* moves, int& numMoves)
{
    numMoves = 0;

    // create various bitboards to help with generation
    uint64_t whitePieces = 0;
    uint64_t blackPieces = 0;

    for (int i = 0; i < 6; ++i)
        whitePieces |= stateStack[stackIndex].bitboards[i];
    
    for (int i = 6; i < 12; ++i)
        blackPieces |= stateStack[stackIndex].bitboards[i];
    
    uint64_t occupied = whitePieces | blackPieces;

    if (getTurn() == WHITE)
    // generate pseudo moves for white
    {
        // pawn moves
        uint64_t bb = (stateStack[stackIndex].bitboards[wPawn] >> 8) & ~occupied;
        uint64_t doublePush = ((bb & RANK_3) >> 8) & ~occupied;
        uint8_t square;

        while (bb)
        {
            square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square + 8;
			if (square < 8)
			{
				moves[numMoves++] = { origin, square, wQueen};
				moves[numMoves++] = { origin, square, wRook };
				moves[numMoves++] = { origin, square, wBishop};
				moves[numMoves++] = { origin, square, wKnight};
            }
			else moves[numMoves++] = { origin, square, EMPTY };
            
        }
        while (doublePush)
		{
			square = lsbIndex(doublePush);
			doublePush &= doublePush - 1;
			uint8_t origin = square + 16;
			moves[numMoves++] = { origin, square, EMPTY };
		}

        // knight moves
		bb = stateStack[stackIndex].bitboards[wKnight];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t knightMoves = knightAttacks[square] & ~whitePieces;
			uint8_t target;
			while (knightMoves)
			{
				target = lsbIndex(knightMoves);
				knightMoves &= knightMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}


        // king moves
		square = lsbIndex(stateStack[stackIndex].bitboards[wKing]);
		bb = kingAttacks[square] & ~whitePieces;
		while (bb)
		{
			uint8_t target = lsbIndex(bb);
			bb &= bb - 1;
			moves[numMoves++] = { square, target, EMPTY };
        }
        
        // castling
        

    }

    else
 // generate pseudo moves for black
    {
        // pawn moves
        uint64_t bb = (stateStack[stackIndex].bitboards[bPawn] << 8) & ~occupied;
        uint64_t doublePush = ((bb & RANK_6) << 8) & ~occupied;
        uint8_t square;

        while (bb)
        {
            square = lsbIndex(bb);
			bb &= bb - 1;
			uint8_t origin = square - 8;
			if (square > 55)
			{
				moves[numMoves++] = { origin, square, bQueen};
				moves[numMoves++] = { origin, square, bRook };
				moves[numMoves++] = { origin, square, bBishop};
				moves[numMoves++] = { origin, square, bKnight};
            }
			else moves[numMoves++] = { origin, square, EMPTY };
            
        }
        while (doublePush)
		{
			square = lsbIndex(doublePush);
			doublePush &= doublePush - 1;
			uint8_t origin = square - 16;
			moves[numMoves++] = { origin, square, EMPTY };
		}

        // knight moves
		bb = stateStack[stackIndex].bitboards[bKnight];
		while (bb)
		{
			square = lsbIndex(bb);
			bb &= bb - 1;

			uint64_t knightMoves = knightAttacks[square] & ~blackPieces;
			uint8_t target;
			while (knightMoves)
			{
				target = lsbIndex(knightMoves);
				knightMoves &= knightMoves - 1;
				moves[numMoves++] = { square, target, EMPTY };
			}
		}


        // king moves
		square = lsbIndex(stateStack[stackIndex].bitboards[bKing]);
		bb = kingAttacks[square] & ~blackPieces;
		while (bb)
		{
			uint8_t target = lsbIndex(bb);
			bb &= bb - 1;
			moves[numMoves++] = { square, target, EMPTY };
        }
        
        // castling
        

    }
}

bool Chessboard::isLegal(const Move& move)
{
    Move moves[218];
    int numMoves;
    pseudoMoves(moves, numMoves);

    for (int i = 0; i < numMoves; ++i)
        if (move == moves[i])
            return true;
    
    return false;
}

bool Chessboard::isAttacked(uint8_t square, uint8_t colour)
{

}

uint8_t Chessboard::wKingSquare()
{
    return lsbIndex(stateStack[stackIndex].bitboards[wKing]);
}

uint8_t Chessboard::bKingSquare()
{
    return lsbIndex(stateStack[stackIndex].bitboards[bKing]);
}


Chessboard::~Chessboard()
{
    delete[] stateStack;    
}