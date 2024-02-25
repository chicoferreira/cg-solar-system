cmake_minimum_required(VERSION 3.25)

project(cg-solar-system LANGUAGES CXX)

add_executable(cg-solar-system src/main.cpp
        src/Engine.cpp
        src/Engine.h
        src/Vec.h
        src/Model.cpp
        src/Model.h
)

find_package(OpenGL REQUIRED)
target_link_libraries(cg-solar-system PUBLIC ${OPENGL_LIBRARIES})
target_include_directories(cg-solar-system PUBLIC ${OPENGL_INCLUDE_DIR})

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(cg-solar-system PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(cg-solar-system PUBLIC imgui::imgui)

find_package(tinyxml2 CONFIG REQUIRED)
target_link_libraries(cg-solar-system PRIVATE tinyxml2::tinyxml2)