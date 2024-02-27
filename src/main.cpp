#include "Engine.h"
#include "World.h"

int main(int, char **)
{
    World world;
    if (!world.LoadFromXml("assets/scenes/plane.xml"))
        return 1;

    Engine engine(world);
    if (!engine.Init())
        return 1;

    engine.Run();

    engine.Shutdown();

    return 0;
}
