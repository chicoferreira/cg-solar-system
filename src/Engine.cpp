#include "Engine.h"

#include <cstdio>
#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

void glfw_error_callback(const int error, const char *description)
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

void SetRenderWireframeMode(const bool enable) { glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL); }

void Engine::renderAxis()
{
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-1000.0, 0.0, 0.0);
    glVertex3f(1000.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, -1000.0, 0.0);
    glVertex3f(0.0, 1000.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, -1000.0);
    glVertex3f(0.0, 0.0, 1000.0);

    glColor3f(1.0, 1.0, 1.0);
    glEnd();
}

void Engine::Render()
{
    renderImGui();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glViewport(0, 0, m_display_w, m_display_h);

    SetRenderWireframeMode(m_wireframe);
    gluPerspective(45.0f, static_cast<float>(m_display_w) / static_cast<float>(m_display_h), 1.0f, 1000.0f);
    gluLookAt(m_camera_pos.x, m_camera_pos.y, m_camera_pos.z, m_camera_looking_at.x, m_camera_looking_at.y,
              m_camera_looking_at.z, 0.0, 1.0, 0.0);

    if (m_render_axis)
        renderAxis();

    for (const auto &m_model : m_models)
    {
        m_model.Render();
    }

    postRenderImGui();
}

void Engine::SetVsync(const bool enable)
{
    m_vsync = enable;
    glfwSwapInterval(enable);
}

void Engine::SetWireframe(const bool enable) { m_wireframe = enable; }

void Engine::Run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        glfwGetFramebufferSize(m_window, &m_display_w, &m_display_h);

        Render();

        glfwMakeContextCurrent(m_window);
        glfwSwapBuffers(m_window);
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

void Engine::AddModel(Model &model) { m_models.push_back(std::move(model)); }

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
        ImGui::Begin("CG Engine");

        if (ImGui::TreeNode("models", "Models (%d)", m_models.size()))
        {
            for (int i = 0; i < m_models.size(); ++i)
            {
                if (ImGui::TreeNode(&m_models[i], "Model #%d", i))
                {
                    if (auto positions = m_models[i].GetVertex();
                        ImGui::TreeNode(&m_models[i].GetVertex(), "Vertices (%d)", positions.size()))
                    {
                        for (const auto &[x, y, z] : positions)
                        {
                            ImGui::Text("x: %.2f, y: %.2f, z: %.2f", x, y, z);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Camera"))
        {
            ImGui::DragFloat3("Position", &m_camera_pos.x, 0.05f);
            ImGui::DragFloat3("Looking At", &m_camera_looking_at.x, 0.05f);

            // Reset
            if (ImGui::Button("Reset"))
            {
                m_camera_pos = {5, 5, 5};
                m_camera_looking_at = {0, 0, 0};
            }

            ImGui::TreePop();
        }

        if (ImGui::Checkbox("VSync", &m_vsync))
        {
            SetVsync(m_vsync);
        }

        if (ImGui::Checkbox("Wireframe", &m_wireframe))
        {
            SetWireframe(m_wireframe);
        }

        ImGui::Checkbox("Render Axis", &m_render_axis);

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    ImGui::Render();
}

void Engine::postRenderImGui() { ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData()); }
