#include "Mat.h"

#define _USE_MATH_DEFINES
#include <math.h>

Mat4f Mat4f::operator*(const Mat4f &other) const
{
    Mat4f result;
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

Mat4f Mat4fTranslate(const float x, const float y, const float z)
{
    return {{{1, 0, 0, x}, {0, 1, 0, y}, {0, 0, 1, z}, {0, 0, 0, 1}}};
}
Mat4f Mat4fScale(const float x, const float y, const float z)
{
    return {{{x, 0, 0, 0}, {0, y, 0, 0}, {0, 0, z, 0}, {0, 0, 0, 1}}};
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
