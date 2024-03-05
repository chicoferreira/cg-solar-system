#ifndef VEC_H
#define VEC_H

#include <math.h>

struct Vec4f;

struct Vec3f
{
    float x, y, z;

    constexpr Vec3f() : x(0), y(0), z(0) {}
    constexpr Vec3f(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    constexpr Vec3f operator+(const Vec3f &other) const { return {x + other.x, y + other.y, z + other.z}; }
    constexpr Vec3f operator-(const Vec3f &other) const { return {x - other.x, y - other.y, z - other.z}; }
    constexpr float operator[](const int i) const { return i == 0 ? x : i == 1 ? y : z; }
    constexpr float &operator[](const int i) { return i == 0 ? x : i == 1 ? y : z; }
    constexpr auto with_y(const float new_y) const { return Vec3f{x, new_y, z}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }

    void ToSpherical(const Vec3f &center, float &radius, float &alpha, float &beta) const
    {
        const Vec3f v = *this - center;
        radius = v.Length();
        alpha = atan2f(v.x, v.z); // arctan(x/z)
        beta = asinf(v.y / radius); // arcsin(y/r)
    }

    Vec4f ToVec4f() const;
};

inline Vec3f Vec3fSpherical(const float radius, const float alpha, const float beta)
{
    return {radius * cosf(beta) * sinf(alpha), radius * sinf(beta), radius * cosf(beta) * cosf(alpha)};
}

inline Vec3f Vec3fPolar(const float radius, const float alpha, const float y = 0)
{
    return {radius * sinf(alpha), y, radius * cosf(alpha)};
}

struct Vec4f
{
    float x = 0, y = 0, z = 0, w = 0;
    constexpr float operator[](const int i) const
    {
        switch (i)
        {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                return w;
        }
    }
    constexpr float &operator[](const int i)
    {
        switch (i)
        {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                return w;
        }
    }
    Vec3f ToVec3f() { return {x, y, z}; }
};

inline Vec4f Vec3f::ToVec4f() const { return {x, y, z, 1}; }

#endif // VEC_H
