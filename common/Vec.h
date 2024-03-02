#ifndef VEC_H
#define VEC_H

#include <math.h>

struct Vec3f
{
    float x, y, z;

    Vec3f() : x(0), y(0), z(0) {}
    Vec3f(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vec3f operator+(const Vec3f &other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3f operator-(const Vec3f &other) const { return {x - other.x, y - other.y, z - other.z}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }

    void ToSpherical(const Vec3f &center, float &radius, float &alpha, float &beta) const
    {
        const Vec3f v = *this - center;
        radius = v.Length();
        alpha = atan2f(v.x, v.z); // arctan(x/z)
        beta = asinf(v.y / radius); // arcsin(y/r)
    };
};

inline Vec3f Vec3fSpherical(const float radius, const float alpha, const float beta)
{
    return {radius * cosf(beta) * sinf(alpha), radius * sinf(beta), radius * cosf(beta) * cosf(alpha)};
}

inline Vec3f Vec3fPolar(const float radius, const float alpha, const float y = 0)
{
    return {radius * sinf(alpha), y, radius * cosf(alpha)};
}

#endif // VEC_H
