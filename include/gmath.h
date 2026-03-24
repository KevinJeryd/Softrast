#ifndef INCLUDE_GMATH_H
#define INCLUDE_GMATH_H

#include <cmath>

namespace GMath
{
    // Types
    struct Vec3
    {
        float x;
        float y;
        float z;
    };

    struct Vec4
    {
        float x;
        float y;
        float z;
        float w;
    };

    struct Mat4
    {
        float m[16];
    };

    // Vertex for local space tris
    struct Vertex
    {
        Vec3 points;
    };

    struct Triangle
    {
        Vertex v[3];
        uint32_t color;
    };

    // Vertex for transformed screen space triangles
    struct ScreenVertex
    {
        int x;
        int y;
        int z;
    };

    struct ScreenTriangle
    {
        ScreenVertex v[3];
    };

    // Mat4 operations
    inline Mat4 operator*(Mat4 const &a, Mat4 const &b)
    {
        Mat4 result{};
        for (int row = 0; row < 4; row++)
        {
            for (int col = 0; col < 4; col++)
            {
                for (int k = 0; k < 4; k++)
                {
                    result.m[row * 4 + col] += a.m[row * 4 + k] * b.m[k * 4 + col];
                }
            }
        }
        return result;
    }

    // Vec4 operations
    inline Vec4 operator*(const Mat4 &m, const Vec4 &v)
    {
        return Vec4{
            m.m[0] * v.x + m.m[1] * v.y + m.m[2] * v.z + m.m[3] * v.w,
            m.m[4] * v.x + m.m[5] * v.y + m.m[6] * v.z + m.m[7] * v.w,
            m.m[8] * v.x + m.m[9] * v.y + m.m[10] * v.z + m.m[11] * v.w,
            m.m[12] * v.x + m.m[13] * v.y + m.m[14] * v.z + m.m[15] * v.w};
    }

    inline Vec4 operator*(Vec4 const &v, float s)
    {
        return Vec4{v.x * s, v.y * s, v.z * s, v.w * s};
    }

    inline Vec4 operator/(Vec4 const &v, float s)
    {
        return Vec4{v.x / s, v.y / s, v.z / s, v.w / s};
    }

    inline Vec4 operator+(Vec4 const &v1, Vec4 const &v2)
    {
        return Vec4{v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w};
    }

    inline Vec4 operator-(Vec4 const &v1, Vec4 const &v2)
    {
        return Vec4{v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
    }

    inline Vec4 operator-(Vec4 const &v)
    {
        return Vec4{-v.x, -v.y, -v.z, -v.w};
    }

    // Vec3 Operations
    inline Vec3 cross(Vec3 const &v1, Vec3 const &v2)
    {
        float x = (v1.y * v2.z) - (v1.z * v2.y);
        float y = (v1.z * v2.x) - (v1.x * v2.z);
        float z = (v1.x * v2.y) - (v1.y * v2.x);
        return Vec3{x, y, z};
    }

    inline Vec3 operator+(Vec3 const &v1, Vec3 const &v2)
    {
        return Vec3{v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    }

    inline Vec3 operator-(Vec3 const &v1, Vec3 const &v2)
    {
        return Vec3{v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    }

    inline Vec3 operator-(Vec3 const &v)
    {
        return Vec3{-v.x, -v.y, -v.z};
    }

    inline Vec3 operator*(Vec3 const &v, float s)
    {
        return Vec3{v.x * s, v.y * s, v.z * s};
    }

    inline float dot(Vec3 const &v1, Vec3 const &v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    inline Vec3 norm(Vec3 const &v)
    {
        float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (length == 0.0f)
            return v;

        return Vec3{v.x / length, v.y / length, v.z / length};
    }

    // Helpers
    inline Mat4 identity()
    {
        Mat4 m{};
        m.m[0] = 1.0f;
        m.m[5] = 1.0f;
        m.m[10] = 1.0f;
        m.m[15] = 1.0f;
        return m;
    }

    inline Mat4 rotationX(float angle)
    {
        Mat4 m = identity();
        m.m[5] = std::cos(angle);
        m.m[6] = -std::sin(angle);
        m.m[9] = std::sin(angle);
        m.m[10] = std::cos(angle);
        return m;
    }

    inline Mat4 rotationY(float angle)
    {
        Mat4 m = identity();
        m.m[0] = std::cos(angle);
        m.m[2] = std::sin(angle);
        m.m[8] = -std::sin(angle);
        m.m[10] = std::cos(angle);
        return m;
    }

    inline Mat4 rotationZ(float angle)
    {
        Mat4 m = identity();
        m.m[0] = std::cos(angle);
        m.m[1] = -std::sin(angle);
        m.m[4] = std::sin(angle);
        m.m[5] = std::cos(angle);
        return m;
    }

    // Transforms
    // Transform from object space to world space.
    inline Mat4 modelMatrix(Vec3 const &t, Vec3 const &r, Vec3 const &s)
    {
        Mat4 translation = identity();
        translation.m[3] = t.x;
        translation.m[7] = t.y;
        translation.m[11] = t.z;

        Mat4 rotation = rotationX(r.x) * rotationY(r.y) * rotationZ(r.z);

        Mat4 scale = identity();
        scale.m[0] = s.x;
        scale.m[5] = s.y;
        scale.m[10] = s.z;

        return translation * rotation * scale;
    }

    // https://www.3dgep.com/understanding-the-view-matrix/
    // Transform from world space to view/camera space
    inline Mat4 viewMatrix(Vec3 const &eye, Vec3 const &target, Vec3 const &up)
    {
        // Build the three axes of the camera's coordinate system
        // Forward points from target back toward the eye (camera looks down -Z)
        Vec3 forward = norm(eye - target);
        Vec3 right = norm(cross(up, forward));
        Vec3 newUp = cross(forward, right);

        // Build the view matrix
        // The upper-left 3x3 is the camera's three axes as rows
        // This re-expresses world coordinates in terms of the camera's coordinate system
        Mat4 m{};
        m.m[0] = right.x;
        m.m[1] = right.y;
        m.m[2] = right.z;
        m.m[3] = -dot(right, eye);
        m.m[4] = newUp.x;
        m.m[5] = newUp.y;
        m.m[6] = newUp.z;
        m.m[7] = -dot(newUp, eye);
        m.m[8] = forward.x;
        m.m[9] = forward.y;
        m.m[10] = forward.z;
        m.m[11] = -dot(forward, eye);
        m.m[12] = 0.0f;
        m.m[13] = 0.0f;
        m.m[14] = 0.0f;
        m.m[15] = 1.0f;

        return m;
    }

    // Transform from view space to clip space
    inline Mat4 projectionMatrix(float fov, float aspect, float near, float far)
    {
        float tanHalfFov = std::tan(fov / 2.0f);

        Mat4 m{};

        // Scale X, Y by FOV
        m.m[0] = 1.0f / (aspect * tanHalfFov);
        m.m[5] = 1.0f / tanHalfFov;

        // Maps z into the [-1, 1] depth range between near and far planes
        m.m[10] = -(far + near) / (far - near);

        // Offset needed to correctly map z to [-1, 1] range
        m.m[11] = -(2.0f * far * near) / (far - near);

        // Copies negated z into w, this is what makes the perspective divide work
        // after this transform you divide x,y,z by w, making far things smaller
        m.m[14] = -1.0f;

        return m;
    }
}

#endif