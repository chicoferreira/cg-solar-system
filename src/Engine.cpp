#include "Engine.h"

#include <cstdio>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

void glfw_error_callback(const int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool Engine::Init()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    m_window = glfwCreateWindow(m_display_w, m_display_h, "CG", nullptr, nullptr);
    if (m_window == nullptr)
        return false;

    glfwMakeContextCurrent(m_window);
    SetVsync(m_vsync);

    initImGui();

    return true;
}

void Engine::Render()
{
    renderImGui();

    glViewport(0, 0, m_display_w, m_display_h);
    glClear(GL_COLOR_BUFFER_BIT);

    postRenderImGui();
}

void Engine::SetVsync(const bool enable)
{
    m_vsync = enable;
    glfwSwapInterval(enable);
}

void Engine::Run()
{
    while (!glfwWindowShouldClose(GetWindow()))
    {
        glfwPollEvents();
        glfwGetFramebufferSize(m_window, &m_display_w, &m_display_h);

        Render();

        glfwMakeContextCurrent(GetWindow());
        glfwSwapBuffers(GetWindow());
    }
}

void Engine::Shutdown() const
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL2_Init();
}


void Engine::renderImGui()
{
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Hello, world!");

        if (ImGui::Checkbox("VSync", &m_vsync))
        {
            SetVsync(m_vsync);
        }

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    ImGui::Render();
}

void Engine::postRenderImGui()
{
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}
