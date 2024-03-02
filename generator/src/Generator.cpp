#include "Generator.h"

#define _USE_MATH_DEFINES
#include "math.h"

std::vector<Vec3f> generator::GeneratePlane(const size_t divisions, const float length)
{
    std::vector<Vec3f> vertex;
    const auto side = length / divisions;

    for (int x = 0; x < divisions; ++x)
    {
        for (int y = 0; y < divisions; ++y)
        {
            const Vec3f top_left = {-length / 2 + x * side, 0, -length / 2 + y * side};
            const Vec3f top_right = {-length / 2 + (x + 1) * side, 0, -length / 2 + y * side};
            const Vec3f bottom_left = {-length / 2 + x * side, 0, -length / 2 + (y + 1) * side};
            const Vec3f bottom_right = {-length / 2 + (x + 1) * side, 0, -length / 2 + (y + 1) * side};

            vertex.push_back(top_left);
            vertex.push_back(bottom_left);
            vertex.push_back(bottom_right);

            vertex.push_back(top_left);
            vertex.push_back(bottom_right);
            vertex.push_back(top_right);
        }
    }

    return vertex;
}

std::vector<Vec3f> generator::GenerateSphere(const float radius, const size_t slices, const size_t stacks)
{
    std::vector<Vec3f> vertex;

    const auto slice_size = 2 * M_PI / slices;
    const auto stack_size = M_PI / stacks;

    for (int slice = 0; slice < slices; ++slice)
    {
        for (int stack = 0; stack < stacks; ++stack)
        {
            // alpha from 0 to 2PI
            // beta from -PI/2 to PI/2
            const Vec3f bottom_left = Vec3fSpherical(radius, slice * slice_size, stack * stack_size - M_PI_2);
            const Vec3f bottom_right = Vec3fSpherical(radius, (slice + 1) * slice_size, stack * stack_size - M_PI_2);
            const Vec3f top_left = Vec3fSpherical(radius, slice * slice_size, (stack + 1) * stack_size - M_PI_2);
            const Vec3f top_right = Vec3fSpherical(radius, (slice + 1) * slice_size, (stack + 1) * stack_size - M_PI_2);

            vertex.push_back(top_left);
            vertex.push_back(bottom_left);
            vertex.push_back(bottom_right);

            vertex.push_back(top_left);
            vertex.push_back(bottom_right);
            vertex.push_back(top_right);
        }
    }

    return vertex;
}

std::vector<Vec3f>
generator::GenerateCone(const float radius, const float height, const size_t slices, const size_t stacks)
{
    std::vector<Vec3f> vertex;

    const auto slice_size = 2 * M_PI / slices;
    const auto stack_size = height / stacks;

    const auto base_middle = Vec3f{0, 0, 0};

    for (int slice = 0; slice < slices; ++slice)
    {
        for (int stack = 0; stack < stacks; ++stack)
        {
            const float current_radius = radius - stack * radius / stacks;
            const float next_radius = radius - (stack + 1) * radius / stacks;

            const Vec3f bottom_left = Vec3fPolar(current_radius, slice * slice_size, stack * stack_size);
            const Vec3f bottom_right = Vec3fPolar(current_radius, (slice + 1) * slice_size, stack * stack_size);
            const Vec3f top_left = Vec3fPolar(next_radius, slice * slice_size, (stack + 1) * stack_size);
            const Vec3f top_right = Vec3fPolar(next_radius, (slice + 1) * slice_size, (stack + 1) * stack_size);

            vertex.push_back(top_left);
            vertex.push_back(bottom_left);
            vertex.push_back(bottom_right);

            vertex.push_back(top_left);
            vertex.push_back(bottom_right);
            vertex.push_back(top_right);
        }

        const Vec3f base_bottom_left = Vec3fPolar(radius, slice * slice_size, 0);
        const Vec3f base_bottom_right = Vec3fPolar(radius, (slice + 1) * slice_size, 0);

        vertex.push_back(base_middle);
        vertex.push_back(base_bottom_right);
        vertex.push_back(base_bottom_left);
    }

    return vertex;
}
