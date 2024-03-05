#include "Engine.h"
#include "World.h"

#include <iostream>

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: engine <scene.xml>" << std::endl;
        return 1;
    }

    const char* file_path = argv[1];
    World world(file_path);
    if (!world.LoadFromXml(file_path))
        return 1;

    Engine engine(world);
    if (!engine.Init())
        return 1;

    engine.Run();

    engine.Shutdown();

    return 0;
}
