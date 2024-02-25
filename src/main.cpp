#include "Engine.h"

// Main code
int main(int, char**)
{
    Engine engine(1280, 720);
    if (!engine.Init())
        return 1;

    const std::vector<Vec3f> positions{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    engine.AddModel(Model(positions));

    engine.Run();

    engine.Shutdown();

    return 0;
}
