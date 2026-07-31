#include <SDL3/SDL.h>
#include <iostream>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Test", 800, 600, 0);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    Uint64 startTime = SDL_GetTicks();
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }

        // auto-close after 3000ms
        if (SDL_GetTicks() - startTime >= 3000) {
            running = false;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}