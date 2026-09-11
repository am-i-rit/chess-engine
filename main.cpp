// main.cpp
#include <iostream>
#include <cmath>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "chessboard.h"
#include "perft.h"
#include "search.h"

Chessboard game; 
Search search;

Colour engineTurn = BLACK;
uint8_t engineDepth = 2;

struct SDLState
{
   SDL_Window* window;
   SDL_Renderer* renderer; 
   int width, height, logW, logH;
   float boardSize, boardX, boardY;
   float squareSize, squareX, squareY;
   int selectedX = -1;
   int selectedY = -1;
   bool hasSelection = false;
   bool boardFlipped = false;
   int promoteX = -1;
   int promoteY = -1;
};

struct Assets
{
    SDL_Texture* pieces[12];
};

Move moveStack[1000];
int moveIndex = 0;

bool initialize(SDLState &state);
std::string getAssetPath(const std::string &relativePath);
bool loadAssets(SDLState &state, Assets &assets);
void cleanupAssets(Assets &assets);
void handleMouseClick(SDLState &state, float windowX, float windowY);
void draw(SDLState &state, Assets &assets);
void drawPieces(SDLState &state, Assets &assets);
bool isWhitePiece(uint8_t piece);
void boardToScreen(const SDLState &state, int boardCol, int boardRow, int& screenCol, int& screenRow);
void render(SDLState &state, Assets &assets);
void cleanup(SDLState &state);

int main(int argc, char* argv[])
{
    // perft and perft divide
    /*
    for (int depth = 1; depth <= 6; ++depth)
    {
        std::cout << "Depth " << depth
                << ": " << perft(game, depth)
                << '\n';
    }
  
    perftDivide(game, 1);
    return 0;
    */

    SDLState state;
    Assets assets;
    state.width = 800;
    state.height = 600;
    state.logW = 640;
    state.logH = 480;

    if (!initialize(state))
    {
        return 1;
    }
    
   if (!loadAssets(state, assets))
    {
        return 1;
    }
    // game loop
    bool running = true;
    while (running)
    {
        // event loop
        SDL_Event event{ 0 };
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    running = false;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    state.width = event.window.data1;
                    state.height = event.window.data2;
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        handleMouseClick(state, event.button.x, event.button.y);
                    }
                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                {
                    switch (event.key.key)
                    {
                        case SDLK_ESCAPE:
                        {
                            running = false;
                            break;
                        }
                        case SDLK_BACKSPACE:
                        {   
                            if (moveIndex > 0)
                            {
                            game.undo();
                            --moveIndex;
                            }
                            break;
                        }
                        case SDLK_SPACE:
                        {
                            state.boardFlipped = !state.boardFlipped;
                            break;
                        }
                    }
                    break;
                }
            }
        }


        render(state, assets);

        if (running &&
            state.promoteY == -1 &&
            game.getTurn() == engineTurn &&
            game.gameResult() == EMPTY)
        {
            Move engineMove =
                search.findBestMove(game, engineDepth);

            if (engineMove.from < 64)
            {
                moveStack[moveIndex] = engineMove;
                ++moveIndex;

                game.move(engineMove);

                std::cout << "Nodes searched: "
                        << search.getNodes() << '\n';
                std::cout << "Evaluation: "
                        <<  '\n';
            }
        }

    }
    cleanupAssets(assets);
    cleanup(state);
    return 0;
}

bool initialize(SDLState &state)
{
    bool initSuccess = true;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        return false;
    }

    // create window
    state.window = SDL_CreateWindow("title", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if (!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        cleanup(state);
        return false;
    }

    // create renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "error creating renderer", nullptr);
        cleanup(state);
        return false;
    }

    // configure presentation
    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    state.boardX = 0.5f * (state.logW - state.logH);
    //state.boardX = 5.0f;
    state.boardY = 0.0f;
    state.boardSize = state.logH;

    state.squareSize = state.boardSize * 0.9f * 0.125f;
    state.squareX = state.boardX + state.boardSize * 0.05f;
    state.squareY = state.boardY + state.boardSize * 0.05f;

    return initSuccess;
}

// resolves a path relative to the executable's own directory,
// so asset loading works regardless of OS or current working directory
std::string getAssetPath(const std::string &relativePath)
{
    std::string path = std::string(SDL_GetBasePath()) + relativePath;
    return path;
}

bool loadAssets(SDLState &state, Assets &assets)
{
    static const char* pieceFilenames[12] = {
        "assets/pieces/white-pawn.png",
        "assets/pieces/white-knight.png",
        "assets/pieces/white-bishop.png",
        "assets/pieces/white-rook.png",
        "assets/pieces/white-queen.png",
        "assets/pieces/white-king.png",
        "assets/pieces/black-pawn.png",
        "assets/pieces/black-knight.png",
        "assets/pieces/black-bishop.png",
        "assets/pieces/black-rook.png",
        "assets/pieces/black-queen.png",
        "assets/pieces/black-king.png"
    };
 
    for (int i = 0; i < 12; ++i)
    {
        assets.pieces[i] = IMG_LoadTexture(state.renderer, getAssetPath(pieceFilenames[i]).c_str());
 
        if (!assets.pieces[i])
        {
            std::cerr << "Failed to load texture: " << pieceFilenames[i] << " - " << SDL_GetError() << std::endl;
            return false;
        }
    }
 
    return true;
}

void cleanupAssets(Assets &assets)
{
    for (int i = 0; i < 12; ++i)
    {
        SDL_DestroyTexture(assets.pieces[i]);
    }
}

void handleMouseClick(SDLState &state, float windowX, float windowY)
{
    float mouseX;
    float mouseY;
    SDL_RenderCoordinatesFromWindow(state.renderer, windowX, windowY, &mouseX, &mouseY);

    int screenCol = floor((mouseX - state.squareX) / state.squareSize);
    int screenRow = floor((mouseY - state.squareY) / state.squareSize);

    // only handle clicks if actually on the board
    if (screenCol < 0 || screenCol > 7 || screenRow < 0 || screenRow > 7)
        return;
    
    int clickedX = state.boardFlipped ? 7 - screenCol : screenCol;
    int clickedY = state.boardFlipped ? 7 - screenRow : screenRow;
  
    // promote a piece if needed
    if (state.promoteY == 0)
    {
        if (clickedX == state.promoteX)
        {
            uint8_t promotion = EMPTY;
            if (clickedY == 0) promotion = wQueen;
            else if (clickedY == 1) promotion = wKnight;
            else if (clickedY == 2) promotion = wRook;
            else if (clickedY == 3) promotion = wBishop;

            if (promotion != EMPTY)
            {
                moveStack[moveIndex - 1].promotion = promotion;
                game.undo();
                game.move(moveStack[moveIndex - 1]);

                state.promoteX = -1;
                state.promoteY = -1;
            }
        }
        return;
    }
    else if (state.promoteY == 7)
    {
        if (clickedX == state.promoteX)
        {
            uint8_t promotion = EMPTY;
            if (clickedY == 7) promotion = bQueen;
            else if (clickedY == 6) promotion = bKnight;
            else if (clickedY == 5) promotion = bRook;
            else if (clickedY == 4) promotion = bBishop;

            if (promotion != EMPTY)
            {
                moveStack[moveIndex - 1].promotion = promotion;
                game.undo();
                game.move(moveStack[moveIndex - 1]);

                state.promoteX = -1;
                state.promoteY = -1;
            }
        }

        return;
    }

    if (game.getTurn() == engineTurn)
    {
        return;
    }

    if (game.gameResult() != EMPTY)
    {
        return;
    }

    uint8_t clickedSquare = clickedY * 8 + clickedX;
    if (!state.hasSelection)
    {
        uint8_t piece = game.getPiece(clickedSquare);

        if (piece == EMPTY) return;
        if (isWhitePiece(piece) != (game.getTurn() == WHITE)) return;

        state.selectedX = clickedX;
        state.selectedY = clickedY;
        state.hasSelection = true;
    }
    else
    {
        uint8_t fromSquare = state.selectedY * 8 + state.selectedX;
        if (clickedSquare == fromSquare)
        {
            state.hasSelection = false;
            return;
        }
        
        Move attemptedMove = {fromSquare, clickedSquare, EMPTY};

        uint8_t movedPiece = game.getPiece(fromSquare);

        bool isPromotion =
            (movedPiece == wPawn && clickedY == 0) ||
            (movedPiece == bPawn && clickedY == 7);

        // use a queen to test if promotion is legal
        Move legalityCheck = attemptedMove;

        if (isPromotion)
        {
            if (movedPiece == wPawn)
                legalityCheck.promotion = wQueen;
            else
                legalityCheck.promotion = bQueen;
        }

        if (!game.isLegal(legalityCheck))
        {
            state.hasSelection = false;
            state.selectedX = -1;
            state.selectedY = -1;
            return;
        }

        moveStack[moveIndex] = attemptedMove;
        game.move(moveStack[moveIndex++]);

        // check for potential promotion
        if ((clickedY == 0 && game.getPiece(clickedSquare) == wPawn) || (clickedY == 7 && game.getPiece(clickedSquare) == bPawn))
        {
            state.promoteX = clickedX;
            state.promoteY = clickedY;
        }
        else
        {
            state.promoteX = -1;
            state.promoteY = -1;
        }

        state.hasSelection = false;
        state.selectedX = -1;
        state.selectedY = -1;
    }
}

void draw(SDLState &state, Assets &assets)
{
    SDL_SetRenderDrawColor(state.renderer, 80, 21, 10, 255);
    SDL_FRect boardRect;
    boardRect.x = state.boardX;
    boardRect.y = state.boardY;
    boardRect.w = state.boardSize;
    boardRect.h = state.boardSize;

    SDL_RenderFillRect(state.renderer, &boardRect);

    // draw the square pattern for the board
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
        {
            if ((x+y) % 2) SDL_SetRenderDrawColor(state.renderer, 181, 136, 99, 255);
            else SDL_SetRenderDrawColor(state.renderer, 240, 217, 181, 255);

            int screenCol, screenRow;
            boardToScreen(state, x, y, screenCol, screenRow);

            SDL_FRect squareRect;
            squareRect.x = (state.squareX + state.squareSize * screenCol);
            squareRect.y = (state.squareY + state.squareSize * screenRow);
            squareRect.w = state.squareSize;
            squareRect.h = state.squareSize;
            SDL_RenderFillRect(state.renderer, &squareRect);
        }

    // highlight selected squares (if any)
    if (state.hasSelection && state.selectedX >= 0 && state.selectedY >= 0 && state.selectedX < 8 && state.selectedY < 8)
    {
        SDL_SetRenderDrawColor(state.renderer, 134, 181, 107, 255);

        int screenCol, screenRow;
        boardToScreen(state, state.selectedX, state.selectedY, screenCol, screenRow);

        SDL_FRect squareRect;
        squareRect.x = (state.squareX + state.squareSize * screenCol);
        squareRect.y = (state.squareY + state.squareSize * screenRow);
        squareRect.w = state.squareSize;
        squareRect.h = state.squareSize;
        SDL_RenderFillRect(state.renderer, &squareRect);
    }

    // draw pieces
    for (int square = 0; square < 64; ++square)
    {
        uint8_t piece = game.getPiece(square);
        if (piece == EMPTY) continue;

        int col = square % 8;
        int row = square / 8;

        int screenCol, screenRow;
        boardToScreen(state, col, row, screenCol, screenRow);

        SDL_FRect destRect;
        destRect.x = state.squareX + state.squareSize * screenCol;
        destRect.y = state.squareY + state.squareSize * screenRow;
        destRect.w = state.squareSize;
        destRect.h = state.squareSize;

        SDL_RenderTexture(state.renderer, assets.pieces[piece], nullptr, &destRect);
    }

    // draw ui for pawn promotions
    if (state.promoteY == 0)
    {
        int screenCol, screenRow;
        boardToScreen(state, state.promoteX, 0, screenCol, screenRow);
        int screenDir;
        if (screenRow == 0) screenDir = 1;
        else screenDir = -1;

        SDL_FRect bg;
        bg.x = state.squareX + state.squareSize * screenCol;
        bg.y = state.squareY + state.squareSize * (screenDir == 1 ? screenRow : screenRow - 3);
        bg.w = state.squareSize;
        bg.h = state.squareSize * 4.0f;
        SDL_SetRenderDrawColor(state.renderer, 160, 220, 255, 255);
        SDL_RenderFillRect(state.renderer, &bg);

        uint8_t options[4] = { wQueen, wKnight, wRook, wBishop };
        for (int i = 0; i < 4; ++i)
        {
            SDL_FRect dest;
            dest.x = state.squareX + state.squareSize * screenCol;
            dest.y = state.squareY + state.squareSize * (screenRow + screenDir * i);
            dest.w = state.squareSize;
            dest.h = state.squareSize;
            SDL_RenderTexture(state.renderer, assets.pieces[options[i]], nullptr, &dest);
        }
    }
    if (state.promoteY == 7)
    {
        int screenCol, screenRow;
        boardToScreen(state, state.promoteX, 7, screenCol, screenRow);
        int screenDir;
        if (screenRow == 0) screenDir = 1;
        else screenDir = -1;

        SDL_FRect bg;
        bg.x = state.squareX + state.squareSize * screenCol;
        bg.y = state.squareY + state.squareSize * (screenDir == 1 ? screenRow : screenRow - 3);
        bg.w = state.squareSize;
        bg.h = state.squareSize * 4.0f;
        SDL_SetRenderDrawColor(state.renderer, 160, 45, 255, 255);
        SDL_RenderFillRect(state.renderer, &bg);

        uint8_t options[4] = { bQueen, bKnight, bRook, bBishop };
        for (int i = 0; i < 4; ++i)
        {
            SDL_FRect dest;
            dest.x = state.squareX + state.squareSize * screenCol;
            dest.y = state.squareY + state.squareSize * (screenRow + screenDir * i);
            dest.w = state.squareSize;
            dest.h = state.squareSize;
            SDL_RenderTexture(state.renderer, assets.pieces[options[i]], nullptr, &dest);
        }
    }

}


bool isWhitePiece(uint8_t piece)
{
    return piece < 6;
}

void boardToScreen(const SDLState &state, int boardCol, int boardRow, int &screenCol, int &screenRow)
{
    if (state.boardFlipped)
    {
        screenCol = 7 - boardCol;
        screenRow = 7 - boardRow;
    }
    else
    {
        screenCol = boardCol;
        screenRow = boardRow;
    }
}

void render(SDLState &state, Assets &assets)
{
    SDL_SetRenderDrawColor(state.renderer, 10, 21, 33, 255);
    SDL_RenderClear(state.renderer);
 
    draw(state, assets);
 
    SDL_RenderPresent(state.renderer);
} 

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}