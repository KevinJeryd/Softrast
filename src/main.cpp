#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>
#include <iostream>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Rasterizer", 800, 600, 0);
    if (!window) {
        std::cout << "window failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cout << "renderer failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        800, 600);
    if (!texture) {
        std::cout << "texture failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::vector<uint32_t> pixels(800 * 600, 0xFF000000);

    for (int i = 0; i < pixels.size(); ++i) {
        pixels[i] = 0xFFFF0000;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT) running = false;

        SDL_UpdateTexture(texture, nullptr, pixels.data(), 800 * sizeof(uint32_t));
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}