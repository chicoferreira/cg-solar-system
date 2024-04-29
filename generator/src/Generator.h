#ifndef GENERATOR_H
#define GENERATOR_H
#include <vector>
#include <cstdint>

#include "Vec.h"

namespace generator
{
    struct GeneratorResult
    {
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> normals;
        std::vector<uint32_t> indexes;
    };

    GeneratorResult GeneratePlane(float length, size_t divisions);
    GeneratorResult GenerateSphere(float radius, size_t slices, size_t stacks);
    GeneratorResult GenerateCone(float radius, float height, size_t slices, size_t stacks);
    GeneratorResult GenerateBox(float length, size_t divisions);
    GeneratorResult GenerateCylinder(float radius, float height, size_t slices);

    bool SaveModel(const GeneratorResult &result, const char *filename);
} // namespace generator


#endif // GENERATOR_H
