#ifndef ENGINE_H
#define ENGINE_H
#include <imgui.h>
#include <GLFW/glfw3.h>


class Engine
{
public:
    Engine(const int display_w, const int display_h) : m_display_w(display_w), m_display_h(display_h)
    {
    }

    bool Init();
    void Render();
    void SetVsync(bool enable);
    GLFWwindow* GetWindow() const { return m_window; }
    void Run();
    void Shutdown() const;

private:
    int m_display_w, m_display_h;

    bool m_vsync{true};

    GLFWwindow* m_window = nullptr;
    ImGuiIO* io = nullptr;

    void initImGui();
    void renderImGui();
    static void postRenderImGui();
};


#endif //ENGINE_H
