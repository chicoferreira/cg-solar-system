#include <iostream>

#include "Vec.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello generator!" << std::endl;
    const Vec3f vec = {1, 2, 3};
    std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
}
