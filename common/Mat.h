#ifndef MAT_H
#define MAT_H

#include <Vec.h>

struct Mat4f
{
    float mat[4][4];
    Mat4f operator*(const Mat4f &other) const;
    Vec4f operator*(const Vec4f &other) const;
};

Mat4f Mat4fTranslate(float x, float y, float z);
Mat4f Mat4fScale(float x, float y, float z);
Mat4f Mat4fRotateX(float angle);
Mat4f Mat4fRotateY(float angle);
Mat4f Mat4fRotateZ(float angle);

// Predefined matrices
constexpr Mat4f Mat4fIdentity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateX_M_PI = {{{1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateX_M_PI_2 = {{{1, 0, 0, 0}, {0, 0, -1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateX_NEGATIVE_M_PI_2 = {{{1, 0, 0, 0}, {0, 0, 1, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateZ_M_PI_2 = {{{0, -1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateZ_NEGATIVE_M_PI_2 = {{{0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

#endif // MAT_H