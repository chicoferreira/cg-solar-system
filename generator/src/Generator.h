#ifndef GENERATOR_H
#define GENERATOR_H
#include <vector>

#include "Vec.h"

namespace generator
{
    std::vector<Vec3f> GeneratePlane(size_t divisions, float length);
    std::vector<Vec3f> GenerateSphere(float radius, size_t slices, size_t stacks);
} // namespace generator


#endif // GENERATOR_H
