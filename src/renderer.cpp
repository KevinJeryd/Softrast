#include "../include/renderer.h"
#include <stdexcept>

namespace Renderer
{
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

        return {window, renderer, texture, pixels};
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

    // https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
    void fillTriangle(GMath::ScreenTriangle const &tri, std::vector<uint32_t> &pixels, int const width, int const height, uint32_t color)
    {
        // Define bounding box
        int minX = std::min(tri.v[0].x, std::min(tri.v[1].x, tri.v[2].x));
        int maxX = std::max(tri.v[0].x, std::max(tri.v[1].x, tri.v[2].x));
        int minY = std::min(tri.v[0].y, std::min(tri.v[1].y, tri.v[2].y));
        int maxY = std::max(tri.v[0].y, std::max(tri.v[1].y, tri.v[2].y));

        // Check against screen bound
        minX = std::max(minX, 0);
        minY = std::max(minY, 0);
        maxX = std::min(maxX, width - 1);
        maxY = std::min(maxY, height - 1);

        // Define delta score constants
        // Edge0
        int dE0_dx = -(tri.v[1].y - tri.v[0].y);
        int dE0_dy = (tri.v[1].x - tri.v[0].x);

        // Edge1
        int dE1_dx = -(tri.v[2].y - tri.v[1].y);
        int dE1_dy = (tri.v[2].x - tri.v[1].x);

        // Edge2
        int dE2_dx = -(tri.v[0].y - tri.v[2].y);
        int dE2_dy = (tri.v[0].x - tri.v[2].x);

        // Define initial score
        int e0_row = (tri.v[1].x - tri.v[0].x) * (minY - tri.v[0].y) - (tri.v[1].y - tri.v[0].y) * (minX - tri.v[0].x);
        int e1_row = (tri.v[2].x - tri.v[1].x) * (minY - tri.v[1].y) - (tri.v[2].y - tri.v[1].y) * (minX - tri.v[1].x);
        int e2_row = (tri.v[0].x - tri.v[2].x) * (minY - tri.v[2].y) - (tri.v[0].y - tri.v[2].y) * (minX - tri.v[2].x);

        for (int y = minY; y <= maxY; ++y)
        {
            int e0 = e0_row;
            int e1 = e1_row;
            int e2 = e2_row;

            for (int x = minX; x <= maxX; ++x)
            {
                bool inside = (e0 >= 0 && e1 >= 0 && e2 >= 0) ||
                              (e0 <= 0 && e1 <= 0 && e2 <= 0);
                if (inside)
                {
                    pixels[y * width + x] = color;
                }

                e0 += dE0_dx;
                e1 += dE1_dx;
                e2 += dE2_dx;
            }

            e0_row += dE0_dy;
            e1_row += dE1_dy;
            e2_row += dE2_dy;
        }
    }

    void drawLineH(std::vector<uint32_t> &pixels, int width, int height, int x0, int y0, int x1, int y1, uint32_t color)
    {
        if (x0 > x1)
        {
            int tempX = x0;
            x0 = x1;
            x1 = tempX;

            int tempY = y0;
            y0 = y1;
            y1 = tempY;
        }

        int dx = x1 - x0;
        int dy = y1 - y0;

        int dir = dy < 0 ? -1 : 1;
        dy *= dir;

        if (dx == 0)
            return;

        int p = 2 * dy - dx;
        int y = y0;

        for (int i = 0; i < dx + 1; ++i)
        {
            if (x0 + i >= 0 && x0 + i < width && y >= 0 && y < height)
                pixels[(y * width) + (x0 + i)] = color;

            if (p >= 0)
            {
                y += dir;
                p = p - 2 * dx;
            }
            p = p + 2 * dy;
        }
    }

    void drawLineV(std::vector<uint32_t> &pixels, int width, int height, int x0, int y0, int x1, int y1, uint32_t color)
    {
        if (y0 > y1)
        {
            int tempX = x0;
            x0 = x1;
            x1 = tempX;

            int tempY = y0;
            y0 = y1;
            y1 = tempY;
        }

        int dx = x1 - x0;
        int dy = y1 - y0;

        int dir = dx < 0 ? -1 : 1;
        dx *= dir;

        if (dy == 0)
            return;

        int p = 2 * dx - dy;
        int x = x0;

        for (int i = 0; i < dy + 1; ++i)
        {
            if (x >= 0 && x < width && y0 + i >= 0 && y0 + i < height)
                pixels[((y0 + i) * width) + x] = color;

            if (p >= 0)
            {
                x += dir;
                p = p - 2 * dy;
            }
            p = p + 2 * dx;
        }
    }

    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    // https://www.youtube.com/watch?v=CceepU1vIKo
    void drawLine(std::vector<uint32_t> &pixels, int width, int height, int x0, int y0, int x1, int y1, uint32_t color)
    {
        if (std::abs(x1 - x0) > std::abs(y1 - y0))
        {
            drawLineH(pixels, width, height, x0, y0, x1, y1, color);
        }
        else
        {
            drawLineV(pixels, width, height, x0, y0, x1, y1, color);
        }
    }

    uint32_t flatShade(GMath::Triangle const &tri, GMath::Vec3 const &lightDir)
    {
        GMath::Vec3 edge1 = tri.v[1].points - tri.v[0].points;
        GMath::Vec3 edge2 = tri.v[2].points - tri.v[0].points;
        GMath::Vec3 normal = GMath::norm(GMath::cross(edge1, edge2));

        float lightAngle = GMath::dot(normal, lightDir);
        lightAngle = (lightAngle + 1) / 2; // Remap from -1, 1 to 0, 1 range
        float brightness = lightAngle * 255;

        // Pack brightness into color
        uint8_t r = ((tri.color >> 16) & 0xFF) * lightAngle;
        uint8_t g = ((tri.color >> 8) & 0xFF) * lightAngle;
        uint8_t b = ((tri.color) & 0xFF) * lightAngle;
        uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;

        return color;
    }
}