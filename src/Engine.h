#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Model.h"
#include "World.h"

class EngineSettings
{
public:
    EngineSettings(bool vsync, bool wireframe, bool render_axis)
        : vsync(vsync), wireframe(wireframe), render_axis(render_axis) {}

    bool vsync{true};
    bool wireframe{false};
    bool render_axis{true};
};

class Engine
{
public:
    explicit Engine(World world) : m_world(std::move(world)) {} 

    EngineSettings settings{true, false, true};

    bool Init();
    void Render();
    void SetVsync(bool enable);
    void SetWireframe(bool enable);
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
