#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "gmath.h"
#include <vector>
#include <SDL3/SDL.h>

namespace Renderer
{
    struct RenderContext
    {
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        std::vector<uint32_t> pixels;
    };

    RenderContext initSDL(int width, int height);
    void fillTriangle(GMath::ScreenTriangle const &tri, std::vector<uint32_t> &pixels, int const width, int const height, uint32_t color);
    void drawLine(std::vector<uint32_t> &pixels, int width, int height, int x0, int y0, int x1, int y1, uint32_t color);
    GMath::ScreenTriangle toScreenSpace(GMath::Triangle const &tri, GMath::Mat4 const &MVP, int winWidth, int winHeight);
    uint32_t flatShade(GMath::Triangle const &tri, GMath::Vec3 const &lightDir);
}

#endif