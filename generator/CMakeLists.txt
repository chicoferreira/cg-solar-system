include_directories(../common)

add_executable(cg-generator src/main.cpp ../common/Vec.h
        src/Generator.cpp
        src/Generator.h
        ../common/Mat.h
        ../common/Mat.cpp
        ../common/World.h
        ../common/WorldSerde.h
        ../common/WorldSerde.cpp
        src/SolarSystem.cpp
        src/SolarSystem.h)

find_package(tinyxml2 CONFIG REQUIRED)
target_link_libraries(cg-generator PRIVATE tinyxml2::tinyxml2)

find_path(RAPIDCSV_INCLUDE_DIRS "rapidcsv.h")
target_include_directories(cg-generator PRIVATE ${RAPIDCSV_INCLUDE_DIRS})