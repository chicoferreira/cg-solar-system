#include <iostream>

#include "Generator.h"
#include "Vec.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello generator!" << std::endl;

    for (const auto fs = generator::GeneratePlane(11, 10); const auto &vec : fs)
    {
        std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
    }
}
