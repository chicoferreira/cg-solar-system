#ifndef GENERATOR_H
#define GENERATOR_H
#include <vector>

#include "Vec.h"

namespace generator
{
    std::vector<Vec3f> GeneratePlane(float length, size_t divisions);
    std::vector<Vec3f> GenerateSphere(float radius, size_t slices, size_t stacks);
    std::vector<Vec3f> GenerateCone(float radius, float height, size_t slices, size_t stacks);
    std::vector<Vec3f> GenerateBox(float length, size_t divisions);
    std::vector<Vec3f> GenerateCylinder(float radius, float height, size_t slices);
    std::vector<Vec3f> GenerateBezier(const char* file_path, size_t tesselation);

    bool SaveModel(const std::vector<Vec3f> &vertex, const char *filename);
} // namespace generator


#endif // GENERATOR_H
