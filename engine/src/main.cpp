#include "Engine.h"
#include "World.h"

int main(int, char **)
{
    const auto world_path = "assets/scenes/sphere_and_plane.xml";
    World world(world_path);
    if (!world.LoadFromXml(world_path))
        return 1;

    Engine engine(world);
    if (!engine.Init())
        return 1;

    engine.Run();

    engine.Shutdown();

    return 0;
}
