#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <vector>
#include "Model.h"

class Engine
{
public:
    Engine(const int display_w, const int display_h, Vec3f camera_pos) :
        m_display_w(display_w), m_display_h(display_h), m_camera_pos(camera_pos)
    {
    }

    bool Init();
    static void renderAxis();
    void Render();
    void SetVsync(bool enable);
    void SetWireframe(bool enable);
    void Run();
    void Shutdown() const;

    void AddModel(Model &model);

private:
    int m_display_w, m_display_h;
    std::vector<Model> m_models;
    Vec3f m_camera_pos;
    Vec3f m_camera_looking_at{0, 0, 0};

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
