#include <iostream>
#include <cmath>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "chessboard.h"

Chessboard game; 

struct SDLState
{
   SDL_Window* window;
   SDL_Renderer* renderer; 
   int width, height, logW, logH;
   float boardSize, boardX, boardY;
   float squareSize, squareX, squareY;
   int selectedX = -1;
   int selectedY = -1;
};

struct Assets
{
    SDL_Texture* pieces[12];
};

bool initialize(SDLState &state);
std::string getAssetPath(const std::string &relativePath);
bool loadAssets(SDLState &state, Assets &assets);
void cleanupAssets(Assets &assets);
void drawBoard(SDLState &state);
void drawPieces(SDLState &state, Assets &assets);
void render(SDLState &state, Assets &assets);
void cleanup(SDLState &state);

int main(int argc, char* argv[])
{
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
                        float mouseX;
                        float mouseY;
                        SDL_RenderCoordinatesFromWindow(state.renderer, event.button.x, event.button.y, &mouseX, &mouseY);
                        
                        state.selectedX = floor((mouseX - state.squareX) / state.squareSize);
                        state.selectedY = floor((mouseY - state.squareY) / state.squareSize);

                        // test x, y on mouse click
                        // std::cout << "x: " << mouseX << ", y: " << mouseY << std::endl;
                        // std::cout << "x: " << state.selectedX << ", y: " << state.selectedY << std::endl;

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
                    }
                    break;
                }
            }
        }
        render(state, assets);
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

void drawBoard(SDLState &state)
{
    SDL_SetRenderDrawColor(state.renderer, 80, 21, 10, 255);
    SDL_FRect boardRect;
    boardRect.x = state.boardX;
    boardRect.y = state.boardY;
    boardRect.w = state.boardSize;
    boardRect.h = state.boardSize;

    SDL_RenderFillRect(state.renderer, &boardRect);

    // draw the board
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
        {
            if ((x+y) % 2) SDL_SetRenderDrawColor(state.renderer, 181, 136, 99, 255);
            else SDL_SetRenderDrawColor(state.renderer, 240, 217, 181, 255);

            SDL_FRect squareRect;
            squareRect.x = (state.squareX + state.squareSize * x);
            squareRect.y = (state.squareY + state.squareSize * y);
            squareRect.w = state.squareSize;
            squareRect.h = state.squareSize;
            SDL_RenderFillRect(state.renderer, &squareRect);
        }
    // highlight selected squares
    if (state.selectedX >= 0 && state.selectedY >= 0 && state.selectedX < 8 && state.selectedY < 8)
    {
        SDL_SetRenderDrawColor(state.renderer, 134, 181, 107, 255);
        SDL_FRect squareRect;
        squareRect.x = (state.squareX + state.squareSize * state.selectedX);
        squareRect.y = (state.squareY + state.squareSize * state.selectedY);
        squareRect.w = state.squareSize;
        squareRect.h = state.squareSize;
        SDL_RenderFillRect(state.renderer, &squareRect);
    }
}

// loops over every square, asks the Chessboard what's there,
// and draws the matching texture if the square isn't empty
void drawPieces(SDLState &state, Assets &assets)
{
    for (int square = 0; square < 64; ++square)
    {
        uint8_t piece = game.getPiece(square);
        if (piece == Empty) continue;
 
        int col = square % 8;
        int row = square / 8;
 
        SDL_FRect destRect;
        destRect.x = state.squareX + state.squareSize * col;
        destRect.y = state.squareY + state.squareSize * row;
        destRect.w = state.squareSize;
        destRect.h = state.squareSize;
 
        SDL_RenderTexture(state.renderer, assets.pieces[piece], nullptr, &destRect);
    }
}

void render(SDLState &state, Assets &assets)
{
    SDL_SetRenderDrawColor(state.renderer, 10, 21, 33, 255);
    SDL_RenderClear(state.renderer);
 
    drawBoard(state);
    drawPieces(state, assets);
 
    SDL_RenderPresent(state.renderer);
} 

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}