#ifndef VEC_H
#define VEC_H

struct Vec3f
{
    float x, y, z;

    Vec3f() : x(0), y(0), z(0) {}
    Vec3f(const float x, const float y, const float z) : x(x), y(y), z(z) {}
};

#endif // VEC_H
