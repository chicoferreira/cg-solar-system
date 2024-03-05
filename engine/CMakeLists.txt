include_directories(../common)

add_executable(cg-solar-system src/main.cpp
        src/Engine.cpp
        src/Engine.h
        src/Model.cpp
        src/Model.h
        src/World.cpp
        src/World.h
        ../common/Vec.h
        ../common/Mat.h
        ../common/Mat.cpp
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

find_package(GLEW REQUIRED)
target_link_libraries(cg-solar-system PRIVATE GLEW::GLEW)