#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Model.h"
#include "World.h"

class EngineSettings
{
public:
    EngineSettings(const bool vsync, const bool wireframe, const bool render_axis, const bool cull_faces) :
        vsync(vsync), wireframe(wireframe), render_axis(render_axis), cull_faces(cull_faces)
    {
    }

    bool vsync;
    bool wireframe;
    bool render_axis;
    bool cull_faces;
};

class Engine
{
public:
    explicit Engine(World world) : m_world(std::move(world)) {}

    EngineSettings settings{
        true, // vsync
        true, // wireframe
        true, // render_axis
        true, // cull_faces
    };

    bool Init();
    void Render();
    void SetVsync(bool enable);
    void SetWireframe(bool enable);
    void SetCullFaces(bool enable);
    void ProcessInput();
    void Run();
    void Shutdown() const;

private:
    World m_world;

    GLFWwindow *m_window = nullptr;
    ImGuiIO *io = nullptr;

    void initImGui();
    void renderImGui();
    static void postRenderImGui();
};

#endif // ENGINE_H
