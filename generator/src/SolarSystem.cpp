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

    std::vector<Vec3f> generatePointsInElipsis(
        float distance_a,
        float distance_b,
        size_t number_of_points,
        float offset_angle = 0.0f,
        float y = 0.0f
    )
    {
        std::vector<Vec3f> points;
        for (int i = 0; i < number_of_points; ++i)
            points.push_back(Vec3fElipse(distance_a, distance_b, offset_angle + i * M_PI * 2 / number_of_points, y));

        return points;
    }

    std::vector<Vec3f>
    generatePointsInCircle(float distance, size_t number_of_points, float offset_angle = 0.0f, float y = 0.0f)
    {
        std::vector<Vec3f> points;
        for (int i = 0; i < number_of_points; ++i)
            points.push_back(Vec3fPolar(distance, offset_angle + i * M_PI * 2 / number_of_points, y));

        return points;
    }

    float randomBetween(float start, float end) { return start + (double)rand() / RAND_MAX * (end - start); }

    float randomRadians() { return randomBetween(0, 2 * M_PI); }

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
        auto comet_id = world.AddModelName("bezier_5.3d");

        constexpr float sun_diameter = 1392700.0f;
        constexpr float sun_rotational_period = 648.0f;
        constexpr float sun_tilt = degrees_to_radians(7.25f);
        const float real_sun_diameter = sun_diameter / scene_scale_factor / sun_size_scale_factor;
        {
            world::WorldGroup sun_group = world::WorldGroup("Sun");
            sun_group.models.push_back({sphere_id, {}});

            sun_group.transformations.AddTransform(world::transform::Scale(Vec3f(real_sun_diameter)));
            sun_group.transformations.AddTransform(world::transform::Rotation(sun_tilt * 2, {1, 0, 0}));
            sun_group.transformations.AddTransform(
                world::transform::RotationWithTime(log2sign(sun_rotational_period) * 5, {0, 1, 0})
            );

            world.GetParentWorldGroup().children.push_back(sun_group);
        }

        for (const auto &planet : planets)
        {
            world::WorldGroup planetery_group = world::WorldGroup("Planetery of " + planet.name);

            const float distance =
                planet.distance_from_sun * 1000000 / scene_scale_factor / planet_distance_scale_factor +
                real_sun_diameter;

            const float planet_actual_translation_delta = log2sign(planet.orbital_period) * 5;

            planetery_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
                planet_actual_translation_delta,
                false,
                generatePointsInCircle(distance, 10, degrees_to_radians(std::rand() % 360))
            ));

            planetery_group.transformations.AddTransform(
                world::transform::Rotation(degrees_to_radians(planet.orbital_inclination * 5), {1, 0, 0})
            );

            world::WorldGroup planet_group = world::WorldGroup("Planet " + planet.name);
            planet_group.models.push_back({sphere_id, {}});

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

                moon_group.models.push_back({real_moon_diameter < 0.01f ? sphere_low_poly_id : sphere_id});

                const auto random_distance_offset = fmod((double)rand() / (RAND_MAX), log2(moon_distance_to_planet));

                moon_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
                    planet_actual_translation_delta / 5 * 2,
                    false,
                    generatePointsInCircle(moon_distance_to_planet + random_distance_offset, 10, randomRadians())
                ));
                moon_group.transformations.AddTransform(
                    world::transform::Scale(Vec3f(std::max(0.005f, real_moon_diameter)))
                );
                moon_group.transformations.AddTransform(
                    world::transform::RotationWithTime(planet_actual_translation_delta / 5, {0, 1, 0})
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

            const auto distance_offset_min = -5.0f * 1000000 / scene_scale_factor / planet_distance_scale_factor;
            const auto distance_offset_max = 5.0f * 1000000 / scene_scale_factor / planet_distance_scale_factor;

            const auto distance_offset_xz = randomBetween(distance_offset_min, distance_offset_max);
            const auto distance_offset_y = randomBetween(distance_offset_min, distance_offset_max);

            asteroid_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
                randomBetween(30, 35),
                true,
                generatePointsInCircle(
                    asteroid_belt_distance_from_sun + distance_offset_xz, 7, randomRadians(), distance_offset_y
                )
            ));

            asteroid_group.transformations.AddTransform(world::transform::Rotation(-M_PI_2, {1, 0, 0}));
            asteroid_group.transformations.AddTransform(world::transform::Scale(Vec3f(900 / scene_scale_factor)));

            asteroid_group.models.push_back({asteroid_id, {}});

            asteroid_belt_group.children.push_back(asteroid_group);
        }

        world.GetParentWorldGroup().children.push_back(asteroid_belt_group);

        world::WorldGroup comet_group = world::WorldGroup("Comet");
        comet_group.models.push_back({comet_id, {}});

        const auto comet_distance_a_from_sun =
            1500.0f * 1000000 / scene_scale_factor / planet_distance_scale_factor + real_sun_diameter;
        const auto comet_distance_b_from_sun =
            1800.0f * 1000000 / scene_scale_factor / planet_distance_scale_factor + real_sun_diameter;

        comet_group.transformations.AddTransform(world::transform::Translation({comet_distance_a_from_sun / 1.2f, 0, 0})
        );
        comet_group.transformations.AddTransform(world::transform::TranslationThroughPoints(
            60.0f,
            true,
            generatePointsInElipsis(comet_distance_a_from_sun, comet_distance_b_from_sun, 10, randomRadians())
        ));
        comet_group.transformations.AddTransform(world::transform::Rotation(-M_PI_2, {1, 0, 0}));
        comet_group.transformations.AddTransform(world::transform::RotationWithTime(20.0f, {1, 0, 0}));

        world.GetParentWorldGroup().children.push_back(comet_group);

        world::serde::SaveWorldToXml(world.GetFilePath().c_str(), world);
    }
} // namespace generator::solarsystem
