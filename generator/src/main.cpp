#include <iostream>

#include "Generator.h"
#include "Vec.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello generator!" << std::endl;

    for (const auto fs = generator::GenerateSphere(2.0f, 10, 10); const auto &vec : fs)
    {
        std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl;
    }
}
