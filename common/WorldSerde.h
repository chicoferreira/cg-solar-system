#ifndef CG_SOLAR_SYSTEM_WORLDSERDE_H
#define CG_SOLAR_SYSTEM_WORLDSERDE_H

#include "World.h"

namespace world::serde
{
    bool LoadWorldFromXml(const char *filename, World &world);
    bool SaveWorldToXml(const char *filename, World &world);
} // namespace engine::world::serde

#endif // CG_SOLAR_SYSTEM_WORLDSERDE_H
