# Rash-bot

A chess engine written from scratch in C++17. The project currently includes an SDL-based graphical interface, bitboard move generation, perft testing, a basic Negamax search, and a minimal UCI executable. It's called Rash-bot because once it's completed, hopefully it will play very aggressive (you could say rash!) chess somewhat akin to the style of Rashid Nezhmetdinov.

## Current features

- Bitboard board representation with separate bitboards for all 12 piece types
- State stack for fast move/undo operations
- Pseudo-legal move generation for pawns, knights, bishops, rooks, queens, and kings
- Castling, en passant, and all four promotion choices
- Legal-move validation by making a move and checking whether the moving side's king is attacked
- Checkmate and stalemate detection
- Perft and perft-divide testing
- Material-based evaluation
- Fixed-depth Negamax search with node counting
- SDL3 GUI with mouse controls, promotion selection, board flipping, and undo
- Minimal UCI mode provided as a separate executable

## Perft results

Move generation has been checked against the standard starting-position perft values:

| Depth | Nodes |
|------:|------:|
| 1 | 20 |
| 2 | 400 |
| 3 | 8,902 |
| 4 | 197,281 |
| 5 | 4,865,609 |
| 6 | 119,060,324 |

## Requirements

- A C++17-compatible compiler
- CMake 3.20 or newer
- SDL3
- SDL3_image

On macOS with Homebrew, the dependencies can be installed with:

```sh
brew install cmake sdl3 sdl3_image
```

## Building

Configure the project once:

```sh
cmake -S . -B build
```

Build both executables:

```sh
cmake --build build
```

Or build only one target:

```sh
cmake --build build --target chess-engine
cmake --build build --target chess-engine-uci
```

## Running the GUI

```sh
./build/chess-engine
```

Controls:

- Click a piece and then its destination square to move.
- Click one of the displayed pieces when choosing a promotion.
- Press `Space` to flip the board.
- Press `Backspace` to undo a move.
- Press `Escape` to quit.

The engine side and fixed search depth are currently configured in `main.cpp` using `engineTurn` and `engineDepth`.

## Running in UCI mode

```sh
./build/chess-engine-uci
```

You can test the interface directly from the terminal:

```text
uci
isready
ucinewgame
position startpos moves e2e4 e7e5
go depth 5
quit
```

The engine should answer the `go` command with a line in this form:

```text
bestmove g1f3
```

The UCI executable can also be selected as an engine in a compatible chess GUI. The current implementation is intended to support the basic fixed-depth workflow; full time management, pondering, and search interruption are not implemented yet.

## Project structure

| File | Purpose |
|---|---|
| `main.cpp` | SDL GUI and human-versus-engine game loop |
| `uci_main.cpp` | Entry point for the command-line UCI executable |
| `uci.cpp` / `uci.h` | UCI command parsing and move conversion |
| `chessboard.cpp` / `chessboard.h` | Position state, move generation, move execution, and legality |
| `search.cpp` / `search.h` | Evaluation, Negamax search, and best-move selection |
| `perft.cpp` / `perft.h` | Perft and perft-divide tests |
| `helper.h` | Bitboard constants and helper functions |
| `assets/` | Piece textures used by the SDL GUI |

## Current limitations and next steps

- Draw tracking is not complete: the half-move clock and Zobrist hash must be updated as moves are made before fifty-move and repetition detection can be enabled reliably.
- The evaluation currently considers material only.
- Search is fixed-depth Negamax without alpha-beta pruning.
- UCI time controls and the `stop` command still need search cancellation support.
- FEN position loading may be added to support `position fen ...`.

Likely next improvements are alpha-beta pruning, move ordering, iterative deepening, proper draw tracking with Zobrist hashing, positional evaluation, and UCI time management.
