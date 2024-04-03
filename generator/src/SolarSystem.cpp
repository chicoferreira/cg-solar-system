#include "SolarSystem.h"
#include "World.h"
#include "WorldSerde.h"
#include "rapidcsv.h"

#include <cstring>

namespace generator::solarsystem
{

    void ConvLiteralBool(const std::string &str, bool &result) { result = !strcmp(str.c_str(), "Yes"); }

    Planet ParsePlanet(const size_t row, rapidcsv::Document planets_csv)
    {
        Planet planet;
        planet.name = planets_csv.GetCell<std::string>(0, row);
        planet.diameter = planets_csv.GetCell<float>(1, row);
        planet.rotation_period = planets_csv.GetCell<float>(2, row);
        planet.distance_from_sun = planets_csv.GetCell<float>(3, row);
        planet.perihelion = planets_csv.GetCell<float>(4, row);
        planet.aphelion = planets_csv.GetCell<float>(5, row);
        planet.orbital_period = planets_csv.GetCell<float>(6, row);
        planet.orbital_velocity = planets_csv.GetCell<float>(7, row);
        planet.orbital_inclination = planets_csv.GetCell<float>(8, row);
        planet.obliquity_to_orbit = planets_csv.GetCell<float>(9, row);
        planet.has_ring_system = planets_csv.GetCell<bool>(10, row, ConvLiteralBool);
        return planet;
    }

    Satellite ParseSatellite(const size_t row, rapidcsv::Document satellites_csv)
    {
        Satellite satellite;
        satellite.planet = satellites_csv.GetCell<std::string>(0, row);
        satellite.name = satellites_csv.GetCell<std::string>(1, row);
        satellite.radius = satellites_csv.GetCell<float>(2, row);
        satellite.albedo = satellites_csv.GetCell<float>(3, row);
        return satellite;
    }

    std::vector<Planet> LoadPlanets(std::string planets_file_path, std::string satellites_file_path)
    {
        // Load using rapidcsv
        rapidcsv::Document planets_csv(planets_file_path, rapidcsv::LabelParams(0, -1));
        rapidcsv::Document satellites_csv(satellites_file_path, rapidcsv::LabelParams(0, -1));

        std::vector<Planet> planets;
        for (size_t i = 0; i < planets_csv.GetRowCount(); i++)
        {
            Planet planet = ParsePlanet(i, planets_csv);
            planets.push_back(planet);
        }

        for (size_t i = 0; i < satellites_csv.GetRowCount(); i++)
        {
            Satellite satellite = ParseSatellite(i, satellites_csv);
            for (auto &planet : planets)
            {
                if (planet.name == satellite.planet)
                {
                    planet.moons.push_back(satellite);
                }
            }
        }

        return planets;
    }

#define SCENE_SIZE_RATIO 1.0 / 5000.0
#define PLANET_DISTANCE_RATIO 1.0 / 1000.0

    void GenerateSolarSystem(float sun_size_scale_factor, float planet_distance_scale_factor, float scene_scale_factor)
    {
        std::vector<Planet> planets = LoadPlanets("assets/planets/planets.csv", "assets/planets/satellites.csv");
        world::World world("assets/scenes/solar_system.xml");

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

        constexpr float sun_diameter = 1392700.0f;
        const float real_sun_diameter = sun_diameter / scene_scale_factor / sun_size_scale_factor;
        {
            world::WorldGroup sun_group;
            sun_group.models.push_back(sphere_id);

            sun_group.transformations.AddTransform(world::transformation::Scale(Vec3f(real_sun_diameter)));

            world.GetParentWorldGroup().children.push_back(sun_group);
        }

        for (const auto &planet : planets)
        {
            world::WorldGroup group;
            group.models.push_back(sphere_id);

            const float random_angle = degrees_to_radians(rand() % 360 + 1);
            const float distance =
                planet.distance_from_sun * 1000000 / scene_scale_factor / planet_distance_scale_factor +
                real_sun_diameter;
            const auto coordinates = Vec3fPolar(distance, random_angle);
            const float diameter = planet.diameter / scene_scale_factor;

            group.transformations.AddTransform(world::transformation::Translation(coordinates));
            group.transformations.AddTransform(world::transformation::Scale(Vec3f(diameter)));
            group.transformations.AddTransform(
                world::transformation::Rotation(degrees_to_radians(planet.orbital_inclination), {1, 0, 0})
            );

//            for (const auto &moon : planet.moons)
//            {
//                world::WorldGroup moon_group;
//                group.models.push_back(sphere_id);
//
//                const float moon_random_angle = degrees_to_radians(rand() % 360 + 1);
//
//                const auto moon_coordinates = Vec3fPolar(planet.orbital_eccentricity)
//
//                group.transformations.AddTransform()
//            }

            world.GetParentWorldGroup().children.push_back(group);
        }

        world::serde::SaveWorldToXml(world.GetFilePath().c_str(), world);
    }
} // namespace generator::solarsystem
