#include "Generator.h"

#define _USE_MATH_DEFINES

#include <filesystem>
#include <fstream>
#include <iostream>
#include <math.h>
#include "Mat.h"

std::vector<Vec3f> generator::GeneratePlane(const float length, const size_t divisions)
{
    std::vector<Vec3f> vertex;
    const auto side = length / divisions;

    for (int x = 0; x < divisions; ++x)
    {
        for (int z = 0; z < divisions; ++z)
        {
            const Vec3f top_left = {-length / 2 + x * side, 0, -length / 2 + z * side};
            const Vec3f top_right = {-length / 2 + (x + 1) * side, 0, -length / 2 + z * side};
            const Vec3f bottom_left = {-length / 2 + x * side, 0, -length / 2 + (z + 1) * side};
            const Vec3f bottom_right = {-length / 2 + (x + 1) * side, 0, -length / 2 + (z + 1) * side};

            vertex.insert(vertex.end(), {top_left, bottom_left, bottom_right});
            vertex.insert(vertex.end(), {top_left, bottom_right, top_right});
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

            // Avoid drawing extra line in bottom stack (it is just one triangle)
            if (stack != 0)
                vertex.insert(vertex.end(), {top_left, bottom_left, bottom_right});

            // Avoid drawing extra line in top stack (it is just one triangle)
            if (stack != stacks - 1)
                vertex.insert(vertex.end(), {top_left, bottom_right, top_right});
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

    constexpr auto base_middle = Vec3f{0, 0, 0};

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

            vertex.insert(vertex.end(), {top_left, bottom_left, bottom_right});
            if (stack != stacks - 1)
                vertex.insert(vertex.end(), {top_left, bottom_right, top_right});
        }

        const Vec3f base_bottom_left = Vec3fPolar(radius, slice * slice_size, 0);
        const Vec3f base_bottom_right = Vec3fPolar(radius, (slice + 1) * slice_size, 0);

        vertex.insert(vertex.end(), {base_middle, base_bottom_right, base_bottom_left});
    }

    return vertex;
}

void applyMat4fTransform(const std::vector<Vec3f> &plane, const Mat4f &transform, std::vector<Vec3f> &result)
{
    for (size_t i = 0; i < plane.size(); ++i)
    {
        result.push_back((transform * plane[i].ToVec4f()).ToVec3f());
    }
}

std::vector<Vec3f> generator::GenerateBox(const float length, const size_t divisions)
{
    std::vector<Vec3f> result;
    const auto plane = GeneratePlane(length, divisions);

    const auto move_up = Mat4fTranslate(0, length / 2, 0);
    const auto move_down = Mat4fTranslate(0, -length / 2, 0) * Mat4fRotateX_M_PI;
    const auto move_left = Mat4fTranslate(-length / 2, 0, 0) * Mat4fRotateZ_M_PI_2;
    const auto move_right = Mat4fTranslate(length / 2, 0, 0) * Mat4fRotateZ_NEGATIVE_M_PI_2;
    const auto move_front = Mat4fTranslate(0, 0, length / 2) * Mat4fRotateX_M_PI_2;
    const auto move_back = Mat4fTranslate(0, 0, -length / 2) * Mat4fRotateX_NEGATIVE_M_PI_2;

    applyMat4fTransform(plane, move_up, result);
    applyMat4fTransform(plane, move_down, result);
    applyMat4fTransform(plane, move_left, result);
    applyMat4fTransform(plane, move_right, result);
    applyMat4fTransform(plane, move_front, result);
    applyMat4fTransform(plane, move_back, result);

    return result;
}

std::vector<Vec3f> generator::GenerateCylinder(const float radius, const float height, const size_t slices)
{
    std::vector<Vec3f> result;

    const float alpha = 2.0f * M_PI / static_cast<float>(slices);

    constexpr auto base_middle = Vec3f(0, 0, 0);
    const auto up_middle = Vec3f(0, height, 0);

    for (int i = 0; i < slices; i++)
    {
        Vec3f base_vertex_left = Vec3fPolar(radius, static_cast<float>(i) * alpha);
        Vec3f base_vertex_right = Vec3fPolar(radius, static_cast<float>(i + 1) * alpha);

        Vec3f up_vertex_left = base_vertex_left.with_y(height);
        Vec3f up_vertex_right = base_vertex_right.with_y(height);

        result.insert(result.end(), {up_middle, up_vertex_left, up_vertex_right});
        result.insert(result.end(), {up_vertex_right, up_vertex_left, base_vertex_right});
        result.insert(result.end(), {base_vertex_left, base_vertex_right, up_vertex_left});
        result.insert(result.end(), {base_middle, base_vertex_right, base_vertex_left});
    }

    return result;
}

Vec3f getBezierPatchPoint(const Mat4f &bezier_patch_matrix, float u, float v)
{
    Vec4f u_vec = {u * u * u, u * u, u, 1};
    Vec4f v_vec = {v * v * v, v * v, v, 1};

    // TODO: u_vec * bezier_matrix * v_vec
    return {0, 0, 0};
} 

std::vector<Vec3f> generator::GenerateBezier(const char* file_path, size_t tesselation)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file " << file_path << std::endl;
        return {};
    }

    size_t num_patches;
    file >> num_patches;

    std::vector<std::vector<size_t>> patches(num_patches, std::vector<size_t>(16));
    for (int i = 0; i < num_patches; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            size_t index;
            file >> index;

            patches[i][j] = index;
        }
    }

    size_t num_points;
    file >> num_points;

    std::vector<Vec3f> control_points(num_points);
    for (int i = 0; i < num_points; ++i)
    {
        float x, y, z;
        file >> x;
        file >> y;
        file >> z;

        control_points[i] = {x, y, z};
    }

    float u = 0.0f, v = 0.0f, delta = 1.0f / tesselation;

    std::vector<Vec3f> result;
    for (auto& patch : patches)
    {
        // TODO: Calculate current patch points and bezier matrix
        Mat4f patch_points;
        Mat4f bezier_patch_matrix = getBezierPatchMatrix(patch_points);

        for (int i = 0; i < tesselation; i++, u += delta)
        {
            for (int j = 0; j < tesselation; j++, v += delta)
            {
                Vec3f p0 = getBezierPatchPoint(bezier_patch_matrix, u, v);
                Vec3f p1 = getBezierPatchPoint(bezier_patch_matrix, u, v + delta);
                Vec3f p2 = getBezierPatchPoint(bezier_patch_matrix, u + delta, v);
                Vec3f p3 = getBezierPatchPoint(bezier_patch_matrix, u + delta, v + delta);

                result.insert(result.end(), {p2, p0, p1});
                result.insert(result.end(), {p1, p3, p2});
            }
            v = 0.0f;
        }
        u = v = 0.0f;
    }

    return result;
}

constexpr std::string_view default_folder = "assets/models/";

bool generator::SaveModel(const std::vector<Vec3f> &vertex, const char *filename)
{
    std::filesystem::path path = default_folder;
    path.append(filename);

    if (!exists(path.parent_path()))
    {
        create_directories(path.parent_path());
    }

    // write to file
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
    {
        std::cout << "Failed to open file " << filename << std::endl;
        return false;
    }

    file << vertex.size() << std::endl;

    for (const auto &vec : vertex)
    {
        file << vec.x << " " << vec.y << " " << vec.z << std::endl;
    }
    return true;
}
