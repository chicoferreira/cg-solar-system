#ifndef CG_SOLAR_SYSTEM_SOLARSYSTEM_H
#define CG_SOLAR_SYSTEM_SOLARSYSTEM_H

#include <string>
#include <vector>

namespace generator::solarsystem
{
    struct Satellite
    {
        std::string planet;
        std::string name;
        float radius;
        float albedo;
    };

    struct Planet
    {
        std::string name;
        float diameter;
        float rotation_period;
        float distance_from_sun;
        float perihelion;
        float aphelion;
        float orbital_period;
        float orbital_velocity;
        float orbital_inclination;
        float obliquity_to_orbit;
        bool has_ring_system;
        std::vector<Satellite> moons;
    };

    void GenerateSolarSystem(
        float sun_size_scale_factor,
        float planet_distance_scale_factor,
        float scene_scale_factor,
        int number_of_asteroids,
        const char* output_file
    );
} // namespace generator::solarsystem


#endif // CG_SOLAR_SYSTEM_SOLARSYSTEM_H
