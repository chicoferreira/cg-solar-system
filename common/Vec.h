#ifndef VEC_H
#define VEC_H

struct Vec3f
{
    float x, y, z;

    Vec3f() : x(0), y(0), z(0) {}
    Vec3f(const float x, const float y, const float z) : x(x), y(y), z(z) {}
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
