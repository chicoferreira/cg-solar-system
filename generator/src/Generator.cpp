#include "Generator.h"

std::vector<Vec3f> generator::GeneratePlane(const size_t divisions, const float length)
{
    const auto side = length / divisions;

    std::vector<Vec3f> vertices;

    for (int x = 0; x < divisions; ++x)
    {
        for (int y = 0; y < divisions; ++y)
        {
            const Vec3f top_left = {-length / 2 + x * side, 0, -length / 2 + y * side};
            const Vec3f top_right = {-length / 2 + (x + 1) * side, 0, -length / 2 + y * side};
            const Vec3f bottom_left = {-length / 2 + x * side, 0, -length / 2 + (y + 1) * side};
            const Vec3f bottom_right = {-length / 2 + (x + 1) * side, 0, -length / 2 + (y + 1) * side};

            vertices.push_back(top_left);
            vertices.push_back(bottom_left);
            vertices.push_back(bottom_right);

            vertices.push_back(top_left);
            vertices.push_back(bottom_right);
            vertices.push_back(top_right);
        }
    }

    return vertices;
}
