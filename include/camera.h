#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include "gmath.h"

namespace Camera {

struct OrbitCamera
{
    float azimuth = 0.0f;
    float elevation = 0.2f;
    float distance = 2.0f;
    float sensitivity = 0.005f;
    float zoomSpeed = 0.2f;
    GMath::Vec3 target{0, 0, 0};

    GMath::Vec3 getEye() const
    {
        float x = target.x + distance * std::cos(elevation) * std::sin(azimuth);
        float y = target.y + distance * std::sin(elevation);
        float z = target.z + distance * std::cos(elevation) * std::cos(azimuth);
        return {x, y, z};
    }

    GMath::Mat4 getViewMatrix() const
    {
        return GMath::viewMatrix(getEye(), target, {0, 1, 0});
    }

    void orbit(float dx, float dy)
    {
        azimuth -= dx * sensitivity;
        elevation += dy * sensitivity;

        float limit = 89.0f * (3.14159f / 180.0f);
        if (elevation > limit)  elevation = limit;
        if (elevation < -limit) elevation = -limit;
    }

    void zoom(float delta)
    {
        distance -= delta * zoomSpeed;
        if (distance < 0.1f) distance = 0.1f;
    }
};

}
#endif