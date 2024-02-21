cmake_minimum_required(VERSION 3.25)

project(cg-solar-system LANGUAGES CXX)

add_executable(cg-solar-system src/main.cpp
        src/test.cpp
        src/test.h
)

find_package(OpenGL REQUIRED)
target_link_libraries(cg-solar-system PRIVATE ${OPENGL_LIBRARIES})
target_include_directories(cg-solar-system PRIVATE ${OPENGL_INCLUDE_DIR})

find_package(GLUT REQUIRED)
target_link_libraries(cg-solar-system PRIVATE GLUT::GLUT)