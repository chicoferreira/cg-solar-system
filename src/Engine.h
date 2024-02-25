#ifndef ENGINE_H
#define ENGINE_H
#include <imgui.h>
#include <vector>
#include <GLFW/glfw3.h>
#include "Model.h"


class Engine
{
public:
    Engine(const int display_w, const int display_h) : m_display_w(display_w), m_display_h(display_h)
    {
    }

    bool Init();
    void Render();
    void SetVsync(bool enable);
    void SetWireframe(bool enable);
    GLFWwindow* GetWindow() const { return m_window; }
    void Run();
    void Shutdown() const;

    void AddModel(Model model);

private:
    int m_display_w, m_display_h;
    std::vector<Model> m_models;

    bool m_vsync{true};
    bool m_wireframe{false};

    GLFWwindow* m_window = nullptr;
    ImGuiIO* io = nullptr;

    void initImGui();
    void renderImGui();
    static void postRenderImGui();
};


#endif //ENGINE_H
