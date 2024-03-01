#ifndef GENERATOR_H
#define GENERATOR_H
#include <vector>

#include "Vec.h"

namespace generator
{
    std::vector<Vec3f> GeneratePlane(size_t divisions, float length);
}


#endif // GENERATOR_H
