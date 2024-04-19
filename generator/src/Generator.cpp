#include "Generator.h"

#define _USE_MATH_DEFINES

#include <filesystem>
#include <fstream>
#include <iostream>
#include <math.h>
#include "Mat.h"

namespace generator
{
    GeneratorResult GeneratePlane(const float length, const size_t divisions)
    {
        std::vector<Vec3f> vertex;
        std::vector<uint32_t> indexes;
        const auto side = length / divisions;

        for (int z = 0; z < divisions + 1; ++z)
        {
            for (int x = 0; x < divisions + 1; ++x)
            {
                vertex.push_back({-length / 2 + x * side, 0, -length / 2 + z * side});
            }
        }

        for (int z = 0; z < divisions; ++z)
        {
            for (int x = 0; x < divisions; ++x)
            {
                const uint32_t top_left_index = x * (divisions + 1) + z;
                const uint32_t top_right_index = top_left_index + 1;
                const uint32_t bottom_left_index = (x + 1) * (divisions + 1) + z;
                const uint32_t bottom_right_index = bottom_left_index + 1;

                indexes.insert(indexes.end(), {top_left_index, bottom_left_index, bottom_right_index});
                indexes.insert(indexes.end(), {top_left_index, bottom_right_index, top_right_index});
            }
        }

        return {vertex, indexes};
    }

    GeneratorResult GenerateSphere(const float radius, const size_t slices, const size_t stacks)
    {
        std::vector<Vec3f> vertex;
        std::vector<uint32_t> indexes;

        const auto slice_size = 2 * M_PI / slices;
        const auto stack_size = M_PI / stacks;

        vertex.push_back(Vec3f(0, radius, 0)); // top
        vertex.push_back(Vec3f(0, -radius, 0)); // bottom

        for (int slice = 0; slice < slices; ++slice)
        {
            for (int stack = 1; stack < stacks; ++stack)
            {
                // alpha from 0 to 2PI
                // beta from -PI/2 to PI/2
                vertex.push_back(Vec3fSpherical(radius, slice * slice_size, stack * stack_size - M_PI_2));
            }
        }

        for (int slice = 0; slice < slices; ++slice)
        {
            for (int stack = 0; stack < stacks; ++stack)
            {
                // 2 the array starts at 2 because of top and bottom already being there
                // (stack - 1) the current stack - 1 because we don't count the bottom vertex
                // (slice * (stacks - 1)) how many slices have we traversed (stacks - 1)
                // because we don't traverse one of the top/bottom vertex
                uint32_t bottom_left_index = 2 + (stack - 1) + (slice * (stacks - 1));
                uint32_t bottom_right_index = bottom_left_index + stacks - 1;

                // Merge vertex when right index completes a full rotation
                if (slice == slices - 1)
                {
                    bottom_right_index = 2 + (stack - 1);
                }

                uint32_t top_left_index = bottom_left_index + 1;
                uint32_t top_right_index = bottom_right_index + 1;

                // Merge bottom vertex
                if (stack == 0)
                {
                    bottom_right_index = 1;
                    bottom_left_index = 1;
                }

                // Merge top vertex
                if (stack == stacks - 1)
                {
                    top_left_index = 0;
                    top_right_index = 0;
                }

                // Avoid drawing extra line in bottom stack (it is just one triangle)
                if (stack != 0)
                    indexes.insert(indexes.end(), {top_left_index, bottom_left_index, bottom_right_index});

                // Avoid drawing extra line in top stack (it is just one triangle)
                if (stack != stacks - 1)
                    indexes.insert(indexes.end(), {top_left_index, bottom_right_index, top_right_index});
            }
        }

        return {vertex, indexes};
    }

    GeneratorResult GenerateCone(const float radius, const float height, const size_t slices, const size_t stacks)
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

        return {vertex};
    }

    void applyMat4fTransform(const std::vector<Vec3f> &plane, const Mat4f &transform, std::vector<Vec3f> &result)
    {
        for (size_t i = 0; i < plane.size(); ++i)
        {
            result.push_back((transform * plane[i].ToVec4f()).ToVec3f());
        }
    }

    GeneratorResult GenerateBox(const float length, const size_t divisions)
    {
        std::vector<Vec3f> result;
        const auto plane = GeneratePlane(length, divisions);

        const auto move_up = Mat4fTranslate(0, length / 2, 0);
        const auto move_down = Mat4fTranslate(0, -length / 2, 0) * Mat4fRotateX_M_PI;
        const auto move_left = Mat4fTranslate(-length / 2, 0, 0) * Mat4fRotateZ_M_PI_2;
        const auto move_right = Mat4fTranslate(length / 2, 0, 0) * Mat4fRotateZ_NEGATIVE_M_PI_2;
        const auto move_front = Mat4fTranslate(0, 0, length / 2) * Mat4fRotateX_M_PI_2;
        const auto move_back = Mat4fTranslate(0, 0, -length / 2) * Mat4fRotateX_NEGATIVE_M_PI_2;

        //        applyMat4fTransform(plane, move_up, result);
        //        applyMat4fTransform(plane, move_down, result);
        //        applyMat4fTransform(plane, move_left, result);
        //        applyMat4fTransform(plane, move_right, result);
        //        applyMat4fTransform(plane, move_front, result);
        //        applyMat4fTransform(plane, move_back, result);

        return {result};
    }

    GeneratorResult GenerateCylinder(const float radius, const float height, const size_t slices)
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

        return {result};
    }

    constexpr std::string_view default_folder = "assets/models/";

    bool SaveModel(const GeneratorResult &model, const char *filename)
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
            std::cout << "Failed to open file " << filename << "\n";
            return false;
        }

        file << model.vertex.size() << " " << model.indexes.size() << "\n";

        for (const auto &vec : model.vertex)
        {
            file << vec.x << " " << vec.y << " " << vec.z << "\n";
        }

        file << "\n";

        for (size_t i = 0; i < model.indexes.size(); ++i)
        {
            file << model.indexes[i] << (((i + 1) % 3 == 0) ? "\n" : " ");
        }

        file.flush();

        return true;
    }

} // namespace generator
