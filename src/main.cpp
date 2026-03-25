#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include "../include/gmath.h"
#include "../include/renderer.h"
#include "../include/objParser.h"
#include "../include/camera.h"

int main(int argc, char *argv[])
{
    std::string path = argc > 1 ? argv[1] : "dragon.obj";
    std::vector<GMath::Triangle> triangles = ObjParser::parseObj(path);
    GMath::Vec3 lightDir = GMath::norm({0, 1, 1});

    constexpr int winWidth = 1200;
    constexpr int winHeight = 800;
    Renderer::RenderContext ctx = Renderer::initSDL(winWidth, winHeight);

    // Scene transform
    GMath::Mat4 model = GMath::modelMatrix({0, 0, 0}, {0, 0, 0}, {3, 3, 3});

    // Projection (constant unless window resizes)
    float fov = 90.0f * (3.14159f / 180.0f);
    float aspect = (float)winWidth / winHeight;
    GMath::Mat4 proj = GMath::projectionMatrix(fov, aspect, 0.1f, 1000.0f);

    Camera::OrbitCamera cam;
    bool running = true;
    bool dragging = false;
    float lastMouseX = 0, lastMouseY = 0;

    while (running)
    {
        // Input
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    dragging = true;
                    lastMouseX = event.button.x;
                    lastMouseY = event.button.y;
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    dragging = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (dragging)
                {
                    cam.orbit(event.motion.x - lastMouseX, event.motion.y - lastMouseY);
                    lastMouseX = event.motion.x;
                    lastMouseY = event.motion.y;
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                cam.zoom(event.wheel.y);
                break;
            }
        }

        // Build MVP
        GMath::Mat4 MVP = proj * cam.getViewMatrix() * model;

        // Clear
        std::fill(ctx.pixels.begin(), ctx.pixels.end(), 0xFF000000);
        std::fill(ctx.depthBuffer.begin(), ctx.depthBuffer.end(), 1.0f);

        // Draw
        for (auto const &tri : triangles)
        {
            GMath::ScreenTriangle screenTri = Renderer::toScreenSpace(tri, MVP, winWidth, winHeight);
            uint32_t color = Renderer::flatShade(tri, lightDir);
            Renderer::fillTriangle(screenTri, ctx.pixels, ctx.depthBuffer, winWidth, winHeight, color);
        }

        // Present
        SDL_UpdateTexture(ctx.texture, nullptr, ctx.pixels.data(), winWidth * sizeof(uint32_t));
        SDL_RenderTexture(ctx.renderer, ctx.texture, nullptr, nullptr);
        SDL_RenderPresent(ctx.renderer);
    }

    SDL_DestroyTexture(ctx.texture);
    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();
    return 0;
}