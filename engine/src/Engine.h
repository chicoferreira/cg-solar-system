#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "Model.h"
#include "World.h"

class EngineSystemEnvironment
{
public:
    std::string glew_version = "unknown";
    std::string glfw_version = "unknown";
    std::string imgui_version = "unknown";
    std::string opengl_version = "unknown";
    std::string gpu_renderer = "unknown";

    EngineSystemEnvironment() = default;
};

class EngineSettings
{
public:
    EngineSettings(
        const size_t mssa_samples,
        const bool mssa,
        const bool vsync,
        const bool wireframe,
        const bool render_axis,
        const bool cull_faces
    ) :
        mssa_samples(mssa_samples), mssa(mssa), vsync(vsync), wireframe(wireframe), render_axis(render_axis),
        cull_faces(cull_faces)
    {
    }

    size_t mssa_samples;
    bool mssa;
    bool vsync;
    bool wireframe;
    bool render_axis;
    bool cull_faces;
};

class Engine
{
public:
    explicit Engine(World world) : m_world(std::move(world)) {}

    bool Init();
    void Render();
    void SetVsync(bool enable);
    void SetWireframe(bool enable);
    void SetCullFaces(bool enable);
    static void SetMssa(bool enable);
    void ProcessInput();
    void Run();
    void Shutdown() const;

private:
    World m_world;

    EngineSettings m_settings{
        8, // mssa_samples
        true, // mssa
        true, // vsync
        true, // wireframe
        true, // render_axis
        true, // cull_faces
    };

    EngineSystemEnvironment m_system_environment;
    ImGuiIO *io = nullptr;

    GLFWwindow *m_window = nullptr;
    void setupEnvironment();

    void initImGui();
    void renderImGui();
    static void postRenderImGui();
};

#endif // ENGINE_H
