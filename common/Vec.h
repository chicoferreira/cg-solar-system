#ifndef VEC_H
#define VEC_H

#define _USE_MATH_DEFINES
#include <math.h>

constexpr float degrees_to_radians(float degrees);
constexpr float radians_to_degrees(float radians);

struct Vec4f;

struct Vec3f
{
    float x, y, z;

    constexpr explicit Vec3f() : x(0), y(0), z(0) {}
    constexpr explicit Vec3f(const float value) : x(value), y(value), z(value) {}
    constexpr Vec3f(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    constexpr Vec3f operator+(const Vec3f &other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3f &operator+=(const Vec3f &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vec3f &operator-=(const Vec3f &other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    constexpr Vec3f operator-(const Vec3f &other) const { return {x - other.x, y - other.y, z - other.z}; }
    constexpr Vec3f operator*(const Vec3f &other) const { return {x * other.x, y * other.y, z * other.z}; }
    constexpr Vec3f operator*(const float length) const { return {x * length, y * length, z * length}; }
    constexpr Vec3f operator/(const float length) const { return {x / length, y / length, z / length}; }
    constexpr float operator[](const int i) const { return i == 0 ? x : i == 1 ? y : z; }
    constexpr float &operator[](const int i) { return i == 0 ? x : i == 1 ? y : z; }
    constexpr Vec3f operator-() { return {-x, -y, -z}; }
    constexpr auto with_y(const float new_y) const { return Vec3f{x, new_y, z}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }

    void ToSpherical(float &radius, float &alpha, float &beta) const
    {
        radius = this->Length();
        alpha = atan2f(this->x, this->z); // arctan(x/z)
        beta = asinf(this->y / radius); // arcsin(y/r)
    }

    Vec3f Normalize() const
    {
        const auto length = Length();
        if (length == 0)
        {
            return {0, 0, 0};
        }
        return *this / length;
    }
    Vec3f Cross(const Vec3f other) const
    {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }
    Vec4f ToVec4f(float w = 1.0f) const;
};

inline Vec3f Vec3fSpherical(const float radius, const float alpha, const float beta)
{
    return {radius * cosf(beta) * sinf(alpha), radius * sinf(beta), radius * cosf(beta) * cosf(alpha)};
}

inline Vec3f Vec3fPolar(const float radius, const float alpha, const float y = 0)
{
    return {radius * sinf(alpha), y, radius * cosf(alpha)};
}

inline Vec3f Vec3fElipse(const float a, const float b, const float alpha, const float y = 0)
{
    return {a * sinf(alpha), y, b * cosf(alpha)};
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
    Vec3f ToVec3f() const { return {x, y, z}; }

    constexpr float matrixMult(Vec4f &other) { return x * other.x + y * other.y + z * other.z + w * other.w; }
};

inline constexpr float degrees_to_radians(const float degrees) { return degrees * M_PI / 180.0f; }

inline constexpr float radians_to_degrees(const float radians) { return radians * 180.0f / M_PI; }

inline Vec4f Vec3f::ToVec4f(float w) const { return {x, y, z, w}; }

#endif // VEC_H
