#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

struct SDLState
{
   SDL_Window *window;
   SDL_Renderer *renderer; 
   int width, height, logW, logH;
   float boardSize, boardX, boardY;
   float squareSize, squareX, squareY;
};

bool initialize(SDLState &state);
void drawBoard(SDLState &state);
void render(SDLState &state);
void cleanup(SDLState &state);

int main(int argc, char *argv[])
{
    SDLState state;
    state.width = 800;
    state.height = 600;
    state.logW = 640;
    state.logH = 480;

    if (!initialize(state))
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

        render(state);
    }
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
    state.boardY = 0.0f;
    state.boardSize = state.logH;

    state.squareSize = state.boardSize * 0.9f * 0.125f;
    state.squareX = state.boardX + state.boardSize * 0.05f;
    state.squareY = state.boardY + state.boardSize * 0.05f;

    return initSuccess;
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
}

void render(SDLState &state)
{
    SDL_SetRenderDrawColor(state.renderer, 10, 21, 33, 255);
    SDL_RenderClear(state.renderer);

    drawBoard(state);

    SDL_RenderPresent(state.renderer);
} 

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}