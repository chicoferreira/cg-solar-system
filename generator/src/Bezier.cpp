#include "Bezier.h"
#include <fstream>
#include "Mat.h"

namespace generator::bezier
{
    std::optional<Surface> LoadSurfaceFromPatchFile(const std::filesystem::path path)
    {
        std::ifstream file(path);

        if (!file.is_open())
            return std::nullopt;


        Surface surface;

        int32_t patch_count;
        file >> patch_count;
        surface.patches.resize(patch_count);

        for (auto &patch : surface.patches)
        {
            for (auto &patch_index : patch)
            {
                if (file.peek() == ',')
                    file.ignore();
                file >> patch_index;
            }
        }

        int32_t vertex_count;
        file >> vertex_count;
        surface.vertex.resize(vertex_count);

        for (auto &vertex : surface.vertex)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (file.peek() == ',')
                    file.ignore();
                file >> vertex[i];
            }
        }

        return surface;
    }

    std::vector<Vec3f> GenerateBezierPatch(const std::array<Vec3f, 16> &control_points, const size_t tesselation_level)
    {
        std::vector<Vec3f> vertex((tesselation_level + 1) * (tesselation_level + 1));

        const Mat4f bezier_matrix{{{-1, 3, -3, 1}, {3, -6, 3, 0}, {-3, 3, 0, 0}, {1, 0, 0, 0}}};

        for (int c = 0; c < 3; ++c)
        {
            const Mat4f points_matrix{
                {{control_points[0][c], control_points[1][c], control_points[2][c], control_points[3][c]},
                 {control_points[4][c], control_points[5][c], control_points[6][c], control_points[7][c]},
                 {control_points[8][c], control_points[9][c], control_points[10][c], control_points[11][c]},
                 {control_points[12][c], control_points[13][c], control_points[14][c], control_points[15][c]}}
            };

            Mat4f intermediate_matrix = bezier_matrix * points_matrix * bezier_matrix;

            for (int i = 0; i <= tesselation_level; ++i)
            {
                for (int j = 0; j <= tesselation_level; ++j)
                {
                    float u = static_cast<float>(i) / tesselation_level;
                    float v = static_cast<float>(j) / tesselation_level;

                    Vec4f u_vector = {u * u * u, u * u, u, 1};
                    Vec4f v_vector = {v * v * v, v * v, v, 1};

                    vertex[i * (tesselation_level + 1) + j][c] = (intermediate_matrix * u_vector).matrixMult(v_vector);
                }
            }
        }

        return vertex;
    }

    generator::GeneratorResult GenerateBezierSurface(const Surface &surface, const size_t tesselation_level)
    {
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> normals;
        std::vector<uint32_t> indexes;

        uint32_t start = 0;

        for (const std::array<uint32_t, 16> &patch : surface.patches)
        {
            std::array<Vec3f, 16> patch_vertex;
            for (int i = 0; i < patch.size(); ++i)
            {
                patch_vertex[i] = surface.vertex[patch[i]];
            }

            const auto result = GenerateBezierPatch(patch_vertex, tesselation_level);
            vertex.insert(vertex.end(), result.begin(), result.end());
            // TODO: Calculate normals

            for (int z = 0; z < tesselation_level; ++z)
            {
                for (int x = 0; x < tesselation_level; ++x)
                {
                    const uint32_t top_left_index = start + z * (tesselation_level + 1) + x;
                    const uint32_t top_right_index = top_left_index + 1;
                    const uint32_t bottom_left_index = start + (z + 1) * (tesselation_level + 1) + x;
                    const uint32_t bottom_right_index = bottom_left_index + 1;

                    indexes.insert(indexes.end(), {top_left_index, bottom_left_index, bottom_right_index});
                    indexes.insert(indexes.end(), {top_left_index, bottom_right_index, top_right_index});
                }
            }

            start += (tesselation_level + 1) * (tesselation_level + 1);
        }

        return {vertex, normals, indexes};
    }
} // namespace generator::bezier
