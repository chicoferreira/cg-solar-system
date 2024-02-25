#include "Engine.h"

// Main code
int main(int, char**)
{
    Engine engine(1280, 720);
    if (!engine.Init())
        return 1;

    engine.Run();

    engine.Shutdown();

    return 0;
}
