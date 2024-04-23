#ifndef CG_SOLAR_SYSTEM_BEZIER_H
#define CG_SOLAR_SYSTEM_BEZIER_H

#include <array>
#include <filesystem>
#include <vector>
#include "Generator.h"
namespace generator::bezier
{
    struct Surface
    {
        std::vector<Vec3f> vertex;
        std::vector<std::array<uint32_t, 16>> patches;
    };

    std::optional<Surface> LoadSurfaceFromPatchFile(const std::filesystem::path path);
    generator::GeneratorResult GenerateBezierSurface(const Surface &surface, const size_t tesselation_level);
} // namespace generator::bezier

#endif // CG_SOLAR_SYSTEM_BEZIER_H
