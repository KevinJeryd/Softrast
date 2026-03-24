#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include "../include/gmath.h"
#include "../include/renderer.h"
#include "../include/objParser.h"

GMath::Vec3 const &eye{0, 0, 2};

// TODO: Don't hardcode values later
GMath::Mat4 createMVPMatrix(int winWidth, int winHeight)
{
    // Setup model matrix
    GMath::Vec3 const &translation{0.3, -0.8, 0};
    GMath::Vec3 const &rotation{0, 0, 0};
    GMath::Vec3 const &scale{1.5, 1.5, 1.5};
    GMath::Mat4 modelMatrix = GMath::modelMatrix(translation, rotation, scale);

    // Setup view matrix
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

int main(int argc, char *argv[])
{
    std::string path = argc > 1 ? argv[1] : "bunny.obj";
    std::vector<GMath::Triangle> triangles = ObjParser::parseObj(path);
    GMath::Vec3 lightDir = GMath::norm({0, 1, 1});

    // Setup SDL
    constexpr int winWidth = 800;
    constexpr int winHeight = 600;
    Renderer::RenderContext ctx = Renderer::initSDL(winWidth, winHeight);

    // Combine to one matrix
    GMath::Mat4 MVP = createMVPMatrix(winWidth, winHeight);

    std::vector<GMath::ScreenTriangle> screenTris;
    for (auto const &tri : triangles)
    {
        screenTris.push_back(Renderer::toScreenSpace(tri, MVP, winWidth, winHeight));
    }

    // Graphics draw loop
    bool running = true;
    while (running)
    {
        std::fill(ctx.pixels.begin(), ctx.pixels.end(), 0xFF000000);
        std::fill(ctx.depthBuffer.begin(), ctx.depthBuffer.end(), std::numeric_limits<float>::max());

        for (int i = 0; i < triangles.size(); i++)
        {
            if (Renderer::backFaceCull(triangles[i], eye))
            {
                uint32_t color = Renderer::flatShade(triangles[i], lightDir);
                Renderer::fillTriangle(screenTris[i], ctx.pixels, ctx.depthBuffer, winWidth, winHeight, color);
            }
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