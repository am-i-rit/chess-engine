// chessboard.cpp
#include "chessboard.h"
#include "helper.h"
#include <cmath>

Chessboard::Chessboard()
{
    stateStack = new boardState[1000];
    reset();

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
                    knightAttacks[squareIndex] |= (uint64_t(1) << targetSquare);
                }
            }

            // diagonal rays
            int diagonaldx[4] = {-1, 1, -1, 1};
            int diagonaldy[4] = {1, 1, -1, -1};

            for (int dir = 0; dir < 4; ++dir)
            {
                diagonalRays[squareIndex][dir] = 0;

                int nx = x;
                int ny = y;

                while (true)
                {
                    nx += diagonaldx[dir];
                    ny += diagonaldy[dir];

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) break;

                    diagonalRays[squareIndex][dir] |= uint64_t(1) << (nx + 8 * ny);
                }             
            }

            // orthogonal rays
            int orthodx[4] = {-1, 1, 0, 0};
            int orthody[4] = {0, 0, 1, -1};

            for (int dir = 0; dir < 4; ++dir)
            {
                orthogonalRays[squareIndex][dir] = 0;

                int nx = x;
                int ny = y;

                while (true)
                {
                    nx += orthodx[dir];
                    ny += orthody[dir];

                    if (nx < 0 || nx > 7 || ny < 0 || ny > 7) break;

                    orthogonalRays[squareIndex][dir] |= uint64_t(1) << (nx + 8 * ny);
                }             
            }


        }
}

void Chessboard::reset()
{
    stackIndex = 0;
    stateStack[0] = {};

    stateStack[0].bitboards[wPawn]   = 0x00ff000000000000;
    stateStack[0].bitboards[wKnight] = 0x4200000000000000;
    stateStack[0].bitboards[wBishop] = 0x2400000000000000;
    stateStack[0].bitboards[wRook]   = 0x8100000000000000;
    stateStack[0].bitboards[wQueen]  = 0x0800000000000000;
    stateStack[0].bitboards[wKing]   = 0x1000000000000000;

    stateStack[0].bitboards[bPawn]   = 0x000000000000ff00;
    stateStack[0].bitboards[bKnight] = 0x0000000000000042;
    stateStack[0].bitboards[bBishop] = 0x0000000000000024;
    stateStack[0].bitboards[bRook]   = 0x0000000000000081;
    stateStack[0].bitboards[bQueen]  = 0x0000000000000008;
    stateStack[0].bitboards[bKing]   = 0x0000000000000010;

    stateStack[0].wKingside = true;
    stateStack[0].wQueenside = true;
    stateStack[0].bKingside = true;
    stateStack[0].bQueenside = true;

    stateStack[0].enpTarget = 0;
    stateStack[0].turn = WHITE;
    stateStack[0].numHalfMoves = 0;
    stateStack[0].zobristHash = 0;
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

    uint8_t current = getPiece(square); 
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

    // capture
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

        // white pawn attacks
        bb = ((stateStack[stackIndex].bitboards[wPawn] & ~FILE_A) >> 9) & (blackPieces | stateStack[stackIndex].enpTarget);
        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;
            uint8_t origin = square + 9;
            if (square < 8)
            {
                moves[numMoves++] = { origin, square, wQueen };
                moves[numMoves++] = { origin, square, wRook };
                moves[numMoves++] = { origin, square, wBishop };
                moves[numMoves++] = { origin, square, wKnight };
            }
            else moves[numMoves++] = { origin, square, EMPTY };
        }

        bb = ((stateStack[stackIndex].bitboards[wPawn] & ~FILE_H) >> 7) & (blackPieces | stateStack[stackIndex].enpTarget);
        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;
            uint8_t origin = square + 7;
            if (square < 8)
            {
                moves[numMoves++] = { origin, square, wQueen };
                moves[numMoves++] = { origin, square, wRook };
                moves[numMoves++] = { origin, square, wBishop };
                moves[numMoves++] = { origin, square, wKnight };
            }
            else moves[numMoves++] = { origin, square, EMPTY };
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
		if (stateStack[stackIndex].wKingside 
            && !(occupied & 0x6000000000000000) 
            && !isAttacked(60, BLACK) 
            && !isAttacked(61, BLACK) 
            && !isAttacked(62, BLACK))
        {
            moves[numMoves++] = { 60, 62, EMPTY };
        }
		if (stateStack[stackIndex].wQueenside 
            && !(occupied & 0x0e00000000000000) 
            && !isAttacked(58, BLACK) 
            && !isAttacked(59, BLACK) 
            && !isAttacked(60, BLACK))
        {
            moves[numMoves++] = { 60, 58, EMPTY };
        }

        // diagonal moves (bishop and queen)
        bb = stateStack[stackIndex].bitboards[wBishop] | stateStack[stackIndex].bitboards[wQueen];
        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;

            uint64_t diagonalMoves = 0;

            for (int dir = 0; dir < 4; ++dir)
            {
                uint64_t ray = diagonalRays[square][dir];
                uint64_t blockers = ray & occupied;

                if (blockers)
                {
                    u_int8_t blockerSquare;
                    if (dir == 0 or dir == 1) 
                        blockerSquare = lsbIndex(blockers);
                    else 
                        blockerSquare = msbIndex(blockers);

                    ray &= ~diagonalRays[blockerSquare][dir];
                }

                diagonalMoves |= ray;
            }

            diagonalMoves &= ~whitePieces;
            uint8_t target;
            while (diagonalMoves)
            {
                target = lsbIndex(diagonalMoves);
                diagonalMoves &= diagonalMoves - 1;
                moves[numMoves++] = {square, target, EMPTY};

            }
        }

        // orthogonal moves (rook and queen)
        bb = stateStack[stackIndex].bitboards[wRook] | stateStack[stackIndex].bitboards[wQueen];
        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;

            uint64_t orthogonalMoves = 0;

            for (int dir = 0; dir < 4; ++dir)
            {
                uint64_t ray = orthogonalRays[square][dir];
                uint64_t blockers = ray & occupied;

                if (blockers)
                {
                    u_int8_t blockerSquare;
                    if (dir == 1 or dir == 2) blockerSquare = lsbIndex(blockers);
                    else blockerSquare = msbIndex(blockers);

                    ray &= ~orthogonalRays[blockerSquare][dir];
                }

                orthogonalMoves |= ray;
            }

            orthogonalMoves &= ~whitePieces;
            uint8_t target;
            while (orthogonalMoves)
            {
                target = lsbIndex(orthogonalMoves);
                orthogonalMoves &= orthogonalMoves - 1;
                moves[numMoves++] = {square, target, EMPTY};

            }
        }
        

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

        // black pawn attacks
        bb = ((stateStack[stackIndex].bitboards[bPawn] & ~FILE_A) << 7) & (whitePieces | stateStack[stackIndex].enpTarget);
        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;
            uint8_t origin = square - 7;
            if (square >= 56)
            {
                moves[numMoves++] = { origin, square, bQueen };
                moves[numMoves++] = { origin, square, bRook };
                moves[numMoves++] = { origin, square, bBishop };
                moves[numMoves++] = { origin, square, bKnight };
            }
            else moves[numMoves++] = { origin, square, EMPTY };
        }

        bb = ((stateStack[stackIndex].bitboards[bPawn] & ~FILE_H) << 9) & (whitePieces | stateStack[stackIndex].enpTarget);
        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;
            uint8_t origin = square - 9;
            if (square >= 56)
            {
                moves[numMoves++] = { origin, square, bQueen };
                moves[numMoves++] = { origin, square, bRook };
                moves[numMoves++] = { origin, square, bBishop };
                moves[numMoves++] = { origin, square, bKnight };
            }
            else moves[numMoves++] = { origin, square, EMPTY };
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
		if (stateStack[stackIndex].bKingside 
            && !(occupied & 0x0000000000000060) 
            && !isAttacked(4, WHITE) 
            && !isAttacked(6, WHITE) 
            && !isAttacked(5, WHITE))
        {
            moves[numMoves++] = { 4, 6, EMPTY };
        }
		if (stateStack[stackIndex].bQueenside 
            && !(occupied & 0x000000000000000e) 
            && !isAttacked(2, WHITE) 
            && !isAttacked(3, WHITE) 
            && !isAttacked(4, WHITE))
        {
            moves[numMoves++] = { 4, 2, EMPTY };
        }

        // diagonal moves (bishop and queen)
        bb = stateStack[stackIndex].bitboards[bBishop] | stateStack[stackIndex].bitboards[bQueen];

        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;

            uint64_t diagonalMoves = 0;

            for (int dir = 0; dir < 4; ++dir)
            {
                uint64_t ray = diagonalRays[square][dir];
                uint64_t blockers = ray & occupied;

                if (blockers)
                {
                    uint8_t blockerSquare;

                    if (dir == 0 || dir == 1)
                        blockerSquare = lsbIndex(blockers);
                    else
                        blockerSquare = msbIndex(blockers);

                    ray &= ~diagonalRays[blockerSquare][dir];
                }

                diagonalMoves |= ray;
            }

            diagonalMoves &= ~blackPieces;

            while (diagonalMoves)
            {
                uint8_t target = lsbIndex(diagonalMoves);
                diagonalMoves &= diagonalMoves - 1;

                moves[numMoves++] = {square, target, EMPTY};
            }
        }

        // orthogonal moves (rook and queen)
        bb = stateStack[stackIndex].bitboards[bRook] | stateStack[stackIndex].bitboards[bQueen];

        while (bb)
        {
            square = lsbIndex(bb);
            bb &= bb - 1;

            uint64_t orthogonalMoves = 0;

            for (int dir = 0; dir < 4; ++dir)
            {
                uint64_t ray = orthogonalRays[square][dir];
                uint64_t blockers = ray & occupied;

                if (blockers)
                {
                    uint8_t blockerSquare;

                    if (dir == 1 || dir == 2)
                        blockerSquare = lsbIndex(blockers);
                    else
                        blockerSquare = msbIndex(blockers);

                    ray &= ~orthogonalRays[blockerSquare][dir];
                }

                orthogonalMoves |= ray;
            }

            orthogonalMoves &= ~blackPieces;

            while (orthogonalMoves)
            {
                uint8_t target = lsbIndex(orthogonalMoves);
                orthogonalMoves &= orthogonalMoves - 1;

                moves[numMoves++] = {square, target, EMPTY};
            }
        }

            }
}

bool Chessboard::isLegal(const Move& move)
{
    // if (isDrawn()) return false;

    Move moves[218]; // 218 is the theoretical max number of legal moves in a position
    int numMoves;
    pseudoMoves(moves, numMoves);

    // makes a move, if the king is attacked then returns false (i.e. illegal move)
    for (int i = 0; i < numMoves; ++i)
    {
        if (move == moves[i])
        {
            this->move(moves[i]);

            bool illegal;

            if (getTurn() == WHITE)
            {
                illegal = isAttacked(bKingSquare(), WHITE);
            }
            else
            {
                illegal = isAttacked(wKingSquare(), BLACK);
            }

            undo();

            if (!illegal)
            {
                return true;
            }
        }
    }
    return false;
}


// function to check whether a square is attacked by a colour
bool Chessboard::isAttacked(uint8_t square, uint8_t colour)
{
    if (colour == WHITE)
    {
        // create a new bitboard with all pieces
        uint64_t board = 0;
        for (int i = 0; i < 12; ++i)
        {
            board |= stateStack[stackIndex].bitboards[i];
        }

        // check if attacked by a pawn
        if ((((stateStack[stackIndex].bitboards[wPawn] & ~FILE_A) >> 9) | ((stateStack[stackIndex].bitboards[wPawn] & ~FILE_H) >> 7)) & (uint64_t(1) << square)) return true;

        // check if attacked by knight
        if (knightAttacks[square] & stateStack[stackIndex].bitboards[wKnight]) return true;

        // check if attacked by a king
    	if (kingAttacks[square] & stateStack[stackIndex].bitboards[wKing]) return true;

		// check if diagonally attacked
		uint64_t attackers = stateStack[stackIndex].bitboards[wQueen] | stateStack[stackIndex].bitboards[wBishop];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = diagonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 1) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

        // check if orthogonally attacked
		attackers = stateStack[stackIndex].bitboards[wQueen] | stateStack[stackIndex].bitboards[wRook];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = orthogonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 1 || i == 2) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}
    }
    else if (colour == BLACK)
    {
        // create a new bitboard with all pieces
        uint64_t board = 0;
        for (int i = 0; i < 12; ++i)
        {
            board |= stateStack[stackIndex].bitboards[i];
        }

        // check if attacked by a pawn
        if ((((stateStack[stackIndex].bitboards[bPawn] & ~FILE_A) << 7) | ((stateStack[stackIndex].bitboards[bPawn] & ~FILE_H) << 9)) & (uint64_t(1) << square)) return true;

        // check if attacked by knight
        if (knightAttacks[square] & stateStack[stackIndex].bitboards[bKnight]) return true;

        // check if attacked by a king
    	if (kingAttacks[square] & stateStack[stackIndex].bitboards[bKing]) return true;

		// check if diagonally attacked
		uint64_t attackers = stateStack[stackIndex].bitboards[bBishop] | stateStack[stackIndex].bitboards[bQueen];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = diagonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 0 || i == 1) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}

        // check if orthogonally attacked
		attackers = stateStack[stackIndex].bitboards[bRook] | stateStack[stackIndex].bitboards[bQueen];
		for (int i = 0; i < 4; ++i)
		{
			uint64_t blockers = orthogonalRays[square][i] & board;
			if (blockers & attackers)
			{
				uint8_t blockerIndex;
				if (i == 1 || i == 2) blockerIndex = lsbIndex(blockers);
				else blockerIndex = msbIndex(blockers);
				if (attackers & (uint64_t(1) << blockerIndex)) return true;
			}
		}
    }
    return false;
}

uint8_t Chessboard::wKingSquare()
{
    return lsbIndex(stateStack[stackIndex].bitboards[wKing]);
}

uint8_t Chessboard::bKingSquare()
{
    return lsbIndex(stateStack[stackIndex].bitboards[bKing]);
}

uint8_t Chessboard::gameResult()
{
    // find all pseudomoves
    Move moves[256];
    int numMoves;
    pseudoMoves(moves, numMoves);

    // check for legal moves
    for (int i = 0; i < numMoves; ++i)
    {
        move(moves[i]);
        bool illegal;

        if (getTurn() == WHITE) 
        {
            // black just moved, so check if blacks king is attacked
            illegal = isAttacked(bKingSquare(), WHITE);
        }
        else 
        {
            illegal = isAttacked(wKingSquare(), BLACK);
        }
        undo();

        // if there are any legal moves, the game isnt over
        if (!illegal) return EMPTY;        
    }
    // no legal moves
    // black wins
    if (getTurn() == WHITE && isAttacked(wKingSquare(), BLACK)) return BLACK;
    // white wins
    if (getTurn() == BLACK && isAttacked(bKingSquare(), WHITE)) return WHITE;
    // draw
    return DRAWN;
}

bool Chessboard::isDrawn()
{
	// 50 move rule
	if (stateStack[stackIndex].numHalfMoves >= 100) return true;

	// repetition
	int n = 0;
	for (int i = stackIndex - 1; i >= 0; --i)
	{
		if (stateStack[stackIndex].zobristHash == stateStack[i].zobristHash)
		{
			++n;
			if (n > 1) return true;
		}
	}

	// insufficient material
	if (stateStack[stackIndex].bitboards[wPawn] || stateStack[stackIndex].bitboards[bPawn] ||
		stateStack[stackIndex].bitboards[wRook] || stateStack[stackIndex].bitboards[bRook] ||
		stateStack[stackIndex].bitboards[wQueen] || stateStack[stackIndex].bitboards[bQueen]) return false;
	uint64_t white_bb = stateStack[stackIndex].bitboards[wBishop] | stateStack[stackIndex].bitboards[wKnight];
	white_bb &= white_bb - 1;
	uint64_t black_bb = stateStack[stackIndex].bitboards[bBishop] | stateStack[stackIndex].bitboards[bKnight];
	black_bb &= black_bb - 1;
	if (white_bb == 0 && black_bb == 0) return true;

	// if no draws are found, then return false
	return false;
}

uint64_t Chessboard::getBitboard(uint8_t piece) const
{
    return stateStack[stackIndex].bitboards[piece];
}

Chessboard::~Chessboard()
{
    delete[] stateStack;    
}