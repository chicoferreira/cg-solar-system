include_directories(../common)

add_executable(cg-generator src/main.cpp ../common/Vec.h
        src/Generator.cpp
        src/Generator.h)