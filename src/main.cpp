#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include "../include/gmath.h"

GMath::Vertex p1{3, 3, -3};
GMath::Vertex p2{-3, 3, -3};
GMath::Vertex p3{0, 0, -3};
GMath::Triangle testTri{p1, p2, p3};

struct RenderContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    std::vector<uint32_t> pixels;
};

RenderContext initSDL(int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Window *window = SDL_CreateWindow("Rasterizer", 800, 600, 0);
    if (!window)
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             800, 600);
    if (!texture)
    {
        throw std::runtime_error(SDL_GetError());
    }

    std::vector<uint32_t> pixels(width * height, 0xFF000000);

    for (int i = 0; i < pixels.size(); ++i)
    {
        pixels[i] = 0xFFFF0000;
    }

    return {window, renderer, texture, pixels};
}

// TODO: Don't hardcode values later
GMath::Mat4 createMVPMatrix(int winWidth, int winHeight)
{
    // Setup model matrix
    GMath::Vec3 const &translation{1, 1, 0};
    GMath::Vec3 const &rotation{0, 0, 0};
    GMath::Vec3 const &scale{1, 1, 1};
    GMath::Mat4 modelMatrix = GMath::modelMatrix(translation, rotation, scale);

    // Setup view matrix
    GMath::Vec3 const &eye{0, 0, 2};
    GMath::Vec3 const &target{0, 0, 0};
    GMath::Vec3 const &up{0, 1, 0};
    GMath::Mat4 viewMatrix = GMath::viewMatrix(eye, target, up);

    // Setup projection matrix
    float fov = 90.0f * (3.14159f / 180.0f);
    float aspect = (float)winWidth / winHeight;
    float near = 0.1f;
    float far = 1000.0f;
    GMath::Mat4 projMatrix = GMath::projectionMatrix(fov, aspect, near, far);

    return projMatrix * viewMatrix * modelMatrix;
}

GMath::ScreenTriangle toScreenSpace(GMath::Triangle const &tri, GMath::Mat4 const &MVP, int winWidth, int winHeight)
{
    GMath::ScreenTriangle screenTri;
    for (int i = 0; i < 3; i++)
    {
        GMath::Vec4 clipPoint = MVP * GMath::Vec4{tri.v[i].points.x, tri.v[i].points.y, tri.v[i].points.z, 1.0f};

        // Perspective divide
        clipPoint = clipPoint / clipPoint.w;

        // Convert clip space to screen space
        int screen_x = (int)((clipPoint.x + 1.0f) / 2.0f * winWidth);
        int screen_y = (int)((1.0f - clipPoint.y) / 2.0f * winHeight);

        screenTri.v[i].x = screen_x;
        screenTri.v[i].y = screen_y;
    }

    return screenTri;
}

int main()
{
    // Setup SDL
    constexpr int winWidth = 800;
    constexpr int winHeight = 600;
    RenderContext ctx = initSDL(winWidth, winHeight);

    // Combine to one matrix
    GMath::Mat4 MVP = createMVPMatrix(winWidth, winHeight);

    GMath::ScreenTriangle screenTri = toScreenSpace(testTri, MVP, winWidth, winHeight);

    // Graphics draw loop
    bool running = true;
    while (running)
    {
        std::fill(ctx.pixels.begin(), ctx.pixels.end(), 0xFF000000);

        for (auto const &screenVert : screenTri.v)
        {
            if (screenVert.x < 0 || screenVert.x >= winWidth)
                continue;
            if (screenVert.y < 0 || screenVert.y >= winHeight)
                continue;
            ctx.pixels[screenVert.y * winWidth + screenVert.x] = 0xFFFF0000;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT)
                running = false;

        SDL_UpdateTexture(ctx.texture, nullptr, ctx.pixels.data(), 800 * sizeof(uint32_t));
        SDL_RenderTexture(ctx.renderer, ctx.texture, nullptr, nullptr);
        SDL_RenderPresent(ctx.renderer);
    }

    SDL_DestroyTexture(ctx.texture);
    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();
    return 0;
}