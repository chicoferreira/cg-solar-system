#include "SolarSystem.h"
#include "World.h"
#include "WorldSerde.h"
#include "rapidcsv.h"

#include <cstring>

#define _USE_MATH_DEFINES
#include <math.h>

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

    std::vector<Vec3f>
    generatePointsInCircle(float distance, size_t number_of_points, float offset_angle = 0.0f, float y = 0.0f)
    {
        std::vector<Vec3f> points;
        for (int i = 0; i < number_of_points; ++i)
            points.push_back(Vec3fPolar(distance, offset_angle + i * M_PI * 2 / number_of_points, y));

        return points;
    }

    double log2sign(double x) { return x < 0 ? -log2(-x) : log2(x); }

    void GenerateSolarSystem(
        float sun_size_scale_factor,
        float planet_distance_scale_factor,
        float scene_scale_factor,
        int number_of_asteroids,
        const char *output_file
    )
    {
        generatePointsInCircle(1, 4);
        std::vector<Planet> planets = LoadPlanets("assets/planets/planets.csv", "assets/planets/satellites.csv");
        world::World world(output_file, "Solar System");

        world.GetWindow() = world::Window(1200, 1000);
        world.GetCamera() = world::Camera(
            {25.0f, 35.0f, 30.0f}, // position
            {0.0f, 7.0f, 0.0f}, // looking_at
            {0.0f, 1.0f, 0.0f}, // up
            60.0f, // fov
            0.1f, // near
            1000.0f // far
        );

        auto sphere_id = world.AddModelName("sphere_1_20_20.3d");
        auto sphere_low_poly_id = world.AddModelName("sphere_1_10_10.3d");
        auto asteroid_id = world.AddModelName("bezier_1.3d");

        constexpr float sun_diameter = 1392700.0f;
        const float real_sun_diameter = sun_diameter / scene_scale_factor / sun_size_scale_factor;
        {
            world::WorldGroup sun_group = world::WorldGroup("Sun");
            sun_group.models.push_back(sphere_id);

            sun_group.transformations.AddTransform(world::transform::Scale(Vec3f(real_sun_diameter)));

            world.GetParentWorldGroup().children.push_back(sun_group);
        }

        for (const auto &planet : planets)
        {
            world::WorldGroup planetery_group = world::WorldGroup("Planetery of " + planet.name);

            const float distance =
                planet.distance_from_sun * 1000000 / scene_scale_factor / planet_distance_scale_factor +
                real_sun_diameter;

            planetery_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
                log2(planet.orbital_period) * 5,
                false,
                generatePointsInCircle(distance, 10, degrees_to_radians(std::rand() % 360))
            ));

            planetery_group.transformations.AddTransform(
                world::transform::Rotation(degrees_to_radians(planet.orbital_inclination * 5), {1, 0, 0})
            );

            world::WorldGroup planet_group = world::WorldGroup("Planet " + planet.name);
            planet_group.models.push_back(sphere_id);

            const float diameter = planet.diameter / scene_scale_factor;

            planet_group.transformations.AddTransform(world::transform::Scale(Vec3f(diameter)));
            planet_group.transformations.AddTransform(
                world::transform::RotationWithTime(log2sign(planet.rotation_period) * 5, {0, 1, 0})
            );

            planetery_group.children.push_back(planet_group);

            float orbital_radius = planet.diameter * 2;

            for (const auto &moon : planet.moons)
            {
                world::WorldGroup moon_group = world::WorldGroup("Moon " + moon.name);

                const float moon_distance_to_planet =
                    orbital_radius * 1000 / scene_scale_factor / planet_distance_scale_factor;

                const auto real_moon_diameter = moon.radius * 2 / scene_scale_factor;

                moon_group.models.push_back(real_moon_diameter < 0.01f ? sphere_low_poly_id : sphere_id);

                const auto random_distance_offset = fmod((double)rand() / (RAND_MAX), log2(moon_distance_to_planet));

                moon_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
                    log2sign(planet.orbital_period) * 2,
                    false,
                    generatePointsInCircle(
                        moon_distance_to_planet + random_distance_offset, 10, degrees_to_radians(rand() % 360 + 1)
                    )
                ));
                moon_group.transformations.AddTransform(
                    world::transform::Scale(Vec3f(std::max(0.005f, real_moon_diameter)))
                );

                planetery_group.children.push_back(moon_group);
            }

            world.GetParentWorldGroup().children.push_back(planetery_group);
        }

        world::WorldGroup asteroid_belt_group = world::WorldGroup("Asteroid Belt");
        const auto asteroid_belt_distance_from_sun =
            300.0f * 1000000 / scene_scale_factor / planet_distance_scale_factor + real_sun_diameter;

        for (int i = 0; i < number_of_asteroids; ++i)
        {
            world::WorldGroup asteroid_group = world::WorldGroup("Asteroid " + std::to_string(i + 1));

            const auto distance_offset_max = 10.0f * 1000000 / scene_scale_factor / planet_distance_scale_factor;

            const auto distance_offset_xz = (double)rand() / RAND_MAX * distance_offset_max;
            const auto distance_offset_y = (double)rand() / RAND_MAX * distance_offset_max;

            asteroid_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
                30.0f + (double)rand() / RAND_MAX * 5,
                true,
                generatePointsInCircle(
                    asteroid_belt_distance_from_sun + distance_offset_xz,
                    7,
                    degrees_to_radians((double)rand() / (RAND_MAX) * 360 + 1),
                    distance_offset_y
                )
            ));

            asteroid_group.transformations.AddTransform(world::transform::Rotation(-M_PI_2, {1, 0, 0}));
            asteroid_group.transformations.AddTransform(world::transform::Scale(Vec3f(900 / scene_scale_factor)));

            asteroid_group.models.push_back(asteroid_id);

            asteroid_belt_group.children.push_back(asteroid_group);
        }

        world.GetParentWorldGroup().children.push_back(asteroid_belt_group);

        world::serde::SaveWorldToXml(world.GetFilePath().c_str(), world);
    }
} // namespace generator::solarsystem
