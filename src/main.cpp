#include <iostream>
#include <optional>

#include "Engine.h"

#define LOAD_MODEL(name, path, format)                                                                                 \
    auto name## = LoadModelFromFile(path, format);                                                                     \
    if (!name##)                                                                                                       \
    {                                                                                                                  \
        std::cerr << "Failed to read model from file\n";                                                               \
        return 1;                                                                                                      \
    }

int main(int, char **)
{
    Engine engine(1280, 720, {5, 5, 5});
    if (!engine.Init())
        return 1;

    LOAD_MODEL(sphere, "assets/models/sphere.3d", ModelLoadFormat::_3D);
    engine.AddModel(sphere.value());

    LOAD_MODEL(plane, "assets/models/plane.3d", ModelLoadFormat::_3D);
    engine.AddModel(plane.value());

    engine.Run();

    engine.Shutdown();

    return 0;
}
