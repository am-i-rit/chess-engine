#pragma once

#include <cstdint>

class Chessboard;

std::uint64_t perft(Chessboard& board, int depth);
void perftDivide(Chessboard& board, int depth);