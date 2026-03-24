#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include "../include/gmath.h"
#include "../include/renderer.h"
#include "../include/objParser.h"

GMath::Vec3 const &eye{0, 0, 2};

struct OrbitCamera
{
    float azimuth = 0.0f;   // horizontal angle (radians)
    float elevation = 0.2f; // vertical angle (radians)
    float distance = 2.0f;  // distance from target
    GMath::Vec3 target{0, 0, 0};

    GMath::Vec3 getEye() const
    {
        float x = target.x + distance * std::cos(elevation) * std::sin(azimuth);
        float y = target.y + distance * std::sin(elevation);
        float z = target.z + distance * std::cos(elevation) * std::cos(azimuth);
        return {x, y, z};
    }
};

int main(int argc, char *argv[])
{
    std::string path = argc > 1 ? argv[1] : "dragon.obj";
    std::vector<GMath::Triangle> triangles = ObjParser::parseObj(path);
    GMath::Vec3 lightDir = GMath::norm({0, 1, 1});

    constexpr int winWidth = 1200;
    constexpr int winHeight = 800;
    Renderer::RenderContext ctx = Renderer::initSDL(winWidth, winHeight);

    OrbitCamera cam;

    bool running = true;
    bool dragging = false;
    float lastMouseX = 0, lastMouseY = 0;

    while (running)
    {
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
                    float dx = event.motion.x - lastMouseX;
                    float dy = event.motion.y - lastMouseY;
                    cam.azimuth -= dx * 0.005f;
                    cam.elevation += dy * 0.005f;

                    // Clamp elevation to avoid gimbal lock at poles
                    float limit = 89.0f * (3.14159f / 180.0f);
                    if (cam.elevation > limit)
                        cam.elevation = limit;
                    if (cam.elevation < -limit)
                        cam.elevation = -limit;

                    lastMouseX = event.motion.x;
                    lastMouseY = event.motion.y;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                cam.distance -= event.wheel.y * 0.2f;
                if (cam.distance < 0.1f)
                    cam.distance = 0.1f;
                break;
            }
        }

        // Rebuild MVP from current camera state
        GMath::Vec3 eye = cam.getEye();

        GMath::Vec3 translation{0, 0, 0};
        GMath::Vec3 rotation{0, 0, 0};
        GMath::Vec3 scale{3, 3, 3};
        GMath::Mat4 model = GMath::modelMatrix(translation, rotation, scale);

        GMath::Vec3 up{0, 1, 0};
        GMath::Mat4 view = GMath::viewMatrix(eye, cam.target, up);

        float fov = 90.0f * (3.14159f / 180.0f);
        float aspect = (float)winWidth / winHeight;
        GMath::Mat4 proj = GMath::projectionMatrix(fov, aspect, 0.1f, 1000.0f);

        GMath::Mat4 MVP = proj * view * model;

        // Clear buffers
        std::fill(ctx.pixels.begin(), ctx.pixels.end(), 0xFF000000);
        std::fill(ctx.depthBuffer.begin(), ctx.depthBuffer.end(), 1.0f);

        // Transform and draw every frame
        for (auto const &tri : triangles)
        {
            GMath::ScreenTriangle screenTri = Renderer::toScreenSpace(tri, MVP, winWidth, winHeight);
            uint32_t color = Renderer::flatShade(tri, lightDir);
            Renderer::fillTriangle(screenTri, ctx.pixels, ctx.depthBuffer, winWidth, winHeight, color);
        }

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