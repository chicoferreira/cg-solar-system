#ifndef MAT_H
#define MAT_H

#include <Vec.h>
#include <vector>

struct Mat4f
{
    float mat[4][4];
    Mat4f operator*(const Mat4f &other) const;
    Vec4f operator*(const Vec4f &other) const;
    Mat4f &operator*=(const Mat4f &mat4_f);
    Mat4f transpose() const;
};

Mat4f Mat4fTranslate(float x, float y, float z);
Mat4f Mat4fScale(float x, float y, float z);
Mat4f Mat4fRotate(float angle, float x, float y, float z);
Mat4f Mat4fRotateX(float angle);
Mat4f Mat4fRotateY(float angle);
Mat4f Mat4fRotateZ(float angle);

void getCatmullRomPoint(float time, std::vector<Vec3f> points, Vec3f &position, Vec3f &derivative);

Mat4f getBezierPatchMatrix(const Mat4f &control_points_matrix);

// Predefined matrices
constexpr Mat4f CatmullRomMatrix = {{{-0.5f, 1.5f, -1.5f, 0.5f}, {1.0f, -2.5f, 2.0f, -0.5f}, {-0.5f, 0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}}};
constexpr Mat4f BezierMatrix = {{{-1, 3, -3, 1}, {3, -6, 3, 0}, {-3, 3, 0, 0}, {1, 0, 0, 0}}};

constexpr Mat4f Mat4fIdentity = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateX_M_PI = {{{1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateX_M_PI_2 = {{{1, 0, 0, 0}, {0, 0, -1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateX_NEGATIVE_M_PI_2 = {{{1, 0, 0, 0}, {0, 0, 1, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateZ_M_PI_2 = {{{0, -1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
constexpr Mat4f Mat4fRotateZ_NEGATIVE_M_PI_2 = {{{0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

#endif // MAT_H
