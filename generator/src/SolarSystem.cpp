#include "SolarSystem.h"
#include "World.h"
#include "WorldSerde.h"

void generator::solarsystem::GenerateSolarSystem()
{
    world::World world("Solar System");

    world.GetWindow() = world::Window(800, 600);
    world.GetCamera() = world::Camera(
        {4.0f, 3.0f, 6.0f}, // position
        {0.0f, 0.0f, 0.0f}, // looking_at
        {0.0f, 1.0f, 0.0f}, // up
        60.0f, // fov
        1.0f, // near
        1000.0f // far
    );

    auto sphere_id = world.AddModelName("sphere_1_8_8.3d");

    world::WorldGroup moon_group(
        {sphere_id},
        world::GroupTransform(
            {world::transformation::Translation({2.0f, 0.0f, 0.0f}), world::transformation::Scale(Vec3f(0.2f))}
        ),
        {}
    );
    world.GetParentWorldGroup() = world::WorldGroup({sphere_id}, {}, {moon_group});

    world::serde::SaveWorldToXml("assets/scenes/solar_system.xml", world);
}
