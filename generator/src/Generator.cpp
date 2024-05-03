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
        std::vector<Vec3f> normals;
        std::vector<Vec2f> tex_coords;
        std::vector<uint32_t> indexes;
        const auto side = length / divisions;

        for (int z = 0; z < divisions + 1; ++z)
        {
            for (int x = 0; x < divisions + 1; ++x)
            {
                vertex.push_back({-length / 2 + x * side, 0, -length / 2 + z * side});
                normals.push_back({0, 1, 0});
                tex_coords.push_back({x * 1.0f / divisions, z * 1.0f / divisions});
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

        return {vertex, normals, tex_coords, indexes};
    }

    GeneratorResult GenerateSphere(const float radius, const size_t slices, const size_t stacks)
    {
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> normals;
        std::vector<Vec2f> tex_coords;
        std::vector<uint32_t> indexes;

        const auto slice_size = 2 * M_PI / slices;
        const auto stack_size = M_PI / stacks;

        for (int slice = 0; slice <= slices; ++slice)
        {
            for (int stack = 0; stack <= stacks; ++stack)
            {
                // alpha from 0 to 2PI
                // beta from -PI/2 to PI/2
                vertex.push_back(Vec3fSpherical(radius, slice * slice_size, stack * stack_size - M_PI_2));
                normals.push_back(Vec3fSpherical(1, slice * slice_size, stack * stack_size - M_PI_2));
                tex_coords.push_back(Vec2f(slice * 1.0f / slices, stack * 1.0f / stacks));
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
                uint32_t bottom_left_index = stack + (slice * (stacks + 1));
                uint32_t bottom_right_index = bottom_left_index + stacks + 1;

                uint32_t top_left_index = bottom_left_index + 1;
                uint32_t top_right_index = bottom_right_index + 1;

                // Avoid drawing extra line in bottom stack (it is just one triangle)
                if (stack != 0)
                    indexes.insert(indexes.end(), {top_left_index, bottom_left_index, bottom_right_index});

                // Avoid drawing extra line in top stack (it is just one triangle)
                if (stack != stacks - 1)
                    indexes.insert(indexes.end(), {top_left_index, bottom_right_index, top_right_index});
            }
        }

        return {vertex, normals, tex_coords, indexes};
    }

    GeneratorResult GenerateCone(const float radius, const float height, const size_t slices, const size_t stacks)
    {
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> normals;
        std::vector<Vec2f> tex_coords;
        std::vector<uint32_t> indexes;

        const auto slice_size = 2 * M_PI / slices;
        const auto stack_size = height / stacks;

        vertex.push_back(Vec3f{0, 0, 0});
        normals.push_back(Vec3f{0, -1, 0});
        tex_coords.push_back(Vec2f{0.5, 0.5});

        const float cone_angle = atan(radius / height);

        for (int slice = 0; slice <= slices; ++slice)
        {
            vertex.push_back(Vec3fPolar(radius, slice * slice_size, 0)); // base vertice
            normals.push_back(Vec3f{0, -1, 0});
            tex_coords.push_back(Vec2f(0.5, 0.5) + Vec2fPolar(0.5, slice * slice_size));

            const float circle_angle = slice * slice_size;
            for (int stack = 0; stack <= stacks; ++stack)
            {
                const float current_radius = radius - stack * radius / stacks;
                vertex.push_back(Vec3fPolar(current_radius, slice * slice_size, stack * stack_size));
                normals.push_back(
                    Vec3f{cos(cone_angle) * sin(circle_angle), sin(cone_angle), cos(cone_angle) * cos(circle_angle)}
                );
                tex_coords.push_back(Vec2f(slice * 1.0f / slices, stack * 1.0f / stacks));
            }
        }

        for (int slice = 0; slice < slices; ++slice)
        {
            for (int stack = 0; stack <= stacks; ++stack) // 0-3
            {
                uint32_t bottom_left_index = 2 + stack + (slice * (stacks + 2));
                uint32_t bottom_right_index = bottom_left_index + stacks + 2;

                uint32_t top_left_index = bottom_left_index + 1;
                uint32_t top_right_index = bottom_right_index + 1;

                indexes.insert(indexes.end(), {top_left_index, bottom_left_index, bottom_right_index});

                // Avoid drawing extra line in top stack (it is just one triangle)
                if (stack != stacks - 1)
                    indexes.insert(indexes.end(), {top_left_index, bottom_right_index, top_right_index});
            }

            uint32_t base_left_index = 1 + (slice * (stacks + 2));
            uint32_t base_right_index = base_left_index + stacks + 2;

            indexes.insert(indexes.end(), {base_right_index, base_left_index, 0}); // base triangle
        }

        return {vertex, normals, tex_coords, indexes};
    }

    GeneratorResult GenerateBox(const float length, const size_t divisions)
    {
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> normals;
        std::vector<Vec2f> tex_coords;
        std::vector<uint32_t> indexes;
        const auto plane = GeneratePlane(length, divisions);
        const auto vertex_size = plane.vertex.size();

        std::vector<Mat4f> translations = {
            Mat4fTranslate(0, length / 2, 0), // up
            Mat4fTranslate(0, -length / 2, 0), // down
            Mat4fTranslate(-length / 2, 0, 0), // left
            Mat4fTranslate(length / 2, 0, 0), // right
            Mat4fTranslate(0, 0, length / 2), // front
            Mat4fTranslate(0, 0, -length / 2) // back
        };

        std::vector<Mat4f> rotations = {
            Mat4fIdentity, // up
            Mat4fRotateX_M_PI, // down
            Mat4fRotateZ_M_PI_2 * Mat4fRotateY_M_PI_2, // left
            Mat4fRotateZ_NEGATIVE_M_PI_2 * Mat4fRotateY_NEGATIVE_M_PI_2, // right
            Mat4fRotateX_M_PI_2 * Mat4fRotateY_M_PI, // front
            Mat4fRotateX_NEGATIVE_M_PI_2 // back
        };

        for (int i = 0; i < translations.size(); ++i)
        {
            for (auto &v : plane.vertex)
            {
                vertex.push_back((translations[i] * rotations[i] * v.ToVec4f()).ToVec3f());
            }

            for (auto &n : plane.normals)
            {
                normals.push_back((rotations[i] * n.ToVec4f()).ToVec3f());
            }

            for (auto &t : plane.tex_coords)
            {
                tex_coords.push_back(t);
            }

            for (auto index : plane.indexes)
            {
                indexes.push_back(index + i * vertex_size);
            }
        }

        return {vertex, normals, tex_coords, indexes};
    }

    GeneratorResult GenerateCylinder(const float radius, const float height, const size_t slices)
    {
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> normals;
        std::vector<Vec2f> tex_coords;
        std::vector<uint32_t> indexes;

        const float alpha = 2.0f * M_PI / static_cast<float>(slices);

        constexpr Vec2f TOP_BASE_MIDDLE_TEX_COORDS = {0.4375, 0.1875};
        constexpr Vec2f BOTTOM_BASE_MIDDLE_TEX_COORDS = {0.8125, 0.1875};
        constexpr float BASE_RADIUS_TEX_COORDS = 0.1875;
        constexpr float SIDE_START_Y_TEX_COORDS = 0.375;

        vertex.push_back(Vec3f(0, 0, 0));
        normals.push_back(Vec3f(0, -1, 0));
        tex_coords.push_back(BOTTOM_BASE_MIDDLE_TEX_COORDS);
        vertex.push_back(Vec3f(0, height, 0));
        normals.push_back(Vec3f(0, 1, 0));
        tex_coords.push_back(TOP_BASE_MIDDLE_TEX_COORDS);

        const uint32_t base_middle = 0;
        const uint32_t up_middle = 1;

        for (int i = 0; i <= slices; i++)
        {
            const uint32_t current_index = vertex.size();

            Vec3f bottom_vertice = Vec3fPolar(radius, static_cast<float>(i) * alpha);
            Vec3f upper_vertice = bottom_vertice.with_y(height);

            vertex.push_back(bottom_vertice); // base
            normals.push_back(Vec3f(0, -1, 0));
            tex_coords.push_back(BOTTOM_BASE_MIDDLE_TEX_COORDS + Vec2fPolar(BASE_RADIUS_TEX_COORDS, i * alpha));

            vertex.push_back(bottom_vertice); // side
            normals.push_back(Vec3fPolar(1, static_cast<float>(i) * alpha));
            tex_coords.push_back(Vec2f(i * 1.0f / slices, SIDE_START_Y_TEX_COORDS));

            vertex.push_back(upper_vertice); // side
            normals.push_back(Vec3fPolar(1, static_cast<float>(i) * alpha));
            tex_coords.push_back(Vec2f(i * 1.0f / slices, 1.0f));

            vertex.push_back(upper_vertice); // top
            normals.push_back(Vec3f(0, 1, 0));
            tex_coords.push_back(TOP_BASE_MIDDLE_TEX_COORDS + Vec2fPolar(BASE_RADIUS_TEX_COORDS, i * alpha));

            const uint32_t bottom_base_vertex_left = current_index;
            const uint32_t bottom_side_vertex_left = current_index + 1;
            const uint32_t upper_side_vertex_left = current_index + 2;
            const uint32_t upper_top_vertex_left = current_index + 3;

            const uint32_t right_start = current_index + 4;

            const uint32_t bottom_base_vertex_right = right_start;
            const uint32_t bottom_side_vertex_right = right_start + 1;
            const uint32_t upper_side_vertex_right = right_start + 2;
            const uint32_t upper_top_vertex_right = right_start + 3;

            indexes.insert(indexes.end(), {up_middle, upper_top_vertex_left, upper_top_vertex_right});
            indexes.insert(indexes.end(), {upper_side_vertex_right, upper_side_vertex_left, bottom_side_vertex_right});
            indexes.insert(indexes.end(), {bottom_side_vertex_left, bottom_side_vertex_right, upper_side_vertex_left});
            indexes.insert(indexes.end(), {base_middle, bottom_base_vertex_right, bottom_base_vertex_left});
        }

        return {vertex, normals, tex_coords, indexes};
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

        for (const auto &vec : model.normals)
        {
            file << vec.x << " " << vec.y << " " << vec.z << "\n";
        }

        file << "\n";

        for (const auto &vec : model.tex_coords)
        {
            file << vec.x << " " << vec.y << "\n";
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
