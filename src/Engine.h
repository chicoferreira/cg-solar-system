#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Model.h"
#include "World.h"

class Engine
{
public:
    explicit Engine(World world) : m_world(std::move(world)) {}

    bool Init();
    static void renderAxis();
    void Render();
    void SetVsync(bool enable);
    void SetWireframe(bool enable);
    void Run();
    void Shutdown() const;
private:
    World m_world;

    bool m_vsync{true};
    bool m_wireframe{false};
    bool m_render_axis{true};

    GLFWwindow *m_window = nullptr;
    ImGuiIO *io = nullptr;

    void initImGui();
    void renderImGui();
    static void postRenderImGui();
};


#endif // ENGINE_H
