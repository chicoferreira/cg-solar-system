#include "Mat.h"

#define _USE_MATH_DEFINES
#include <math.h>

Mat4f Mat4f::operator*(const Mat4f &other) const
{
    Mat4f result{};
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result.mat[i][j] = 0;
            for (int k = 0; k < 4; ++k)
            {
                result.mat[i][j] += mat[i][k] * other.mat[k][j];
            }
        }
    }
    return result;
}

Vec4f Mat4f::operator*(const Vec4f &other) const
{
    Vec4f result;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result[i] += mat[i][j] * other[j];
        }
    }
    return result;
}

Mat4f &Mat4f::operator*=(const Mat4f &mat4_f)
{
    *this = *this * mat4_f;
    return *this;
}

Mat4f Mat4f::transpose() const
{
    Mat4f result{};
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result.mat[i][j] = mat[j][i];
        }
    }
    return result;
}

Mat4f Mat4fTranslate(const float x, const float y, const float z)
{
    return {{{1, 0, 0, x}, {0, 1, 0, y}, {0, 0, 1, z}, {0, 0, 0, 1}}};
}

Mat4f Mat4fScale(const float x, const float y, const float z)
{
    return {{{x, 0, 0, 0}, {0, y, 0, 0}, {0, 0, z, 0}, {0, 0, 0, 1}}};
}

Mat4f Mat4fRotate(const float angle, const float x, const float y, const float z)
{
    const float cosa = cosf(angle);
    const float sina = sinf(angle);

    return {
        {{
            x * x + (1 - x * x) * cosa,
            x * y * (1 - cosa) - z * sina,
            x * z * (1 - cosa) + y * sina,
            0,
         },
         {
            x * y * (1 - cosa) + z * sina,
            y * y + (1 - y * y) * cosa,
            y * z * (1 - cosa) - x * sina,
            0,
         },
         {
            x * z * (1 - cosa) - y * sina,
            y * z * (1 - cosa) + x * sina,
            z * z + (1 - z * z) * cosa,
            0,
         },
         {0, 0, 0, 1}}
    };
}

Mat4f Mat4fRotateX(const float angle)
{
    const auto c = cosf(angle);
    const auto s = sinf(angle);
    return {{{1, 0, 0, 0}, {0, c, -s, 0}, {0, s, c, 0}, {0, 0, 0, 1}}};
}

Mat4f Mat4fRotateY(const float angle)
{
    const auto c = cosf(angle);
    const auto s = sinf(angle);
    return {{{c, 0, s, 0}, {0, 1, 0, 0}, {-s, 0, c, 0}, {0, 0, 0, 1}}};
}

Mat4f Mat4fRotateZ(const float angle)
{
    const auto c = cosf(angle);
    const auto s = sinf(angle);
    return {{{c, -s, 0, 0}, {s, c, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

void getCatmullRomPointSegment(
    float time,
    const Vec3f &p0,
    const Vec3f &p1,
    const Vec3f &p2,
    const Vec3f &p3,
    Vec3f &position,
    Vec3f &derivative
)
{

    Vec4f time_vector = {time * time * time, time * time, time, 1};
    Vec4f time_derivative_vector = {3 * time * time, 2 * time, 1, 0};

    for (int i = 0; i < 3; ++i)
    {
        Vec4f points = {p0[i], p1[i], p2[i], p3[i]};
        Vec4f result = CatmullRomMatrix * points;

        // position[0] = x, position[1] = y, position[2] = z
        position[i] = time_vector.matrixMult(result);
        derivative[i] = time_derivative_vector.matrixMult(result);
    }
}

void getCatmullRomPoint(float time, std::vector<Vec3f> points, Vec3f &position, Vec3f &derivative)
{
    size_t points_size = points.size();

    float t = time * points_size;
    int index = floor(t);
    float time_in_segment = t - index;

    const auto &p0 = points[(index + points_size - 1) % points_size];
    const auto &p1 = points[(index) % points_size];
    const auto &p2 = points[(index + 1) % points_size];
    const auto &p3 = points[(index + 2) % points_size];

    getCatmullRomPointSegment(time_in_segment, p0, p1, p2, p3, position, derivative);
}

Mat4f getBezierPatchMatrix(const Mat4f &control_points_matrix)
{
    // BezierMatrix = BezierMatrix.transpose()
    return control_points_matrix * BezierMatrix * BezierMatrix;
}
