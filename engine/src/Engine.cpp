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

    const int width = m_world.GetWindow().width;
    const int height = m_world.GetWindow().height;

    m_window = glfwCreateWindow(width, height, "CG", nullptr, nullptr);
    if (m_window == nullptr)
        return false;

    glfwMakeContextCurrent(m_window);
    SetVsync(settings.vsync);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    initImGui();

    return true;
}

void SetRenderWireframeMode(const bool enable) { glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL); }

void renderAxis()
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

void renderCamera(const Camera &camera, const Window &window)
{
    glViewport(0, 0, window.width, window.height);

    const float aspect = static_cast<float>(window.width) / static_cast<float>(window.height);
    gluPerspective(camera.fov, aspect, camera.near, camera.far);
    gluLookAt(
        camera.position.x,
        camera.position.y,
        camera.position.z,
        camera.looking_at.x,
        camera.looking_at.y,
        camera.looking_at.z,
        camera.up.x,
        camera.up.y,
        camera.up.z
    );
}

void renderModel(Model &model)
{
    glBegin(GL_TRIANGLES);

    for (const auto &[x, y, z] : model.GetVertex())
    {
        glVertex3f(x, y, z);
    }

    glEnd();
}

void renderGroup(WorldGroup &group)
{
    for (auto &model : group.models)
    {
        renderModel(model);
    }

    for (auto &child : group.children)
    {
        renderGroup(child);
    }
}

void Engine::Render()
{
    renderImGui();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    renderCamera(m_world.GetCamera(), m_world.GetWindow());

    if (settings.render_axis)
        renderAxis();

    SetRenderWireframeMode(settings.wireframe);

    renderGroup(m_world.GetParentWorldGroup());

    postRenderImGui();
}

void Engine::SetVsync(const bool enable)
{
    settings.vsync = enable;
    glfwSwapInterval(enable);
}

void Engine::SetWireframe(const bool enable) { settings.wireframe = enable; }

void Engine::Run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        glfwGetFramebufferSize(m_window, &m_world.GetWindow().width, &m_world.GetWindow().height);

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

void renderImGuiWorldGroupMenu(WorldGroup &world_group)
{
    auto &models = world_group.models;
    if (ImGui::TreeNode("Group"))
    {
        if (ImGui::TreeNode("models", "Models (%d)", models.size()))
        {
            for (int i = 0; i < models.size(); ++i)
            {
                if (ImGui::TreeNode(&models[i], "Model #%d", i))
                {
                    if (auto positions = models[i].GetVertex();
                        ImGui::TreeNode(&models[i].GetVertex(), "Vertices (%d)", positions.size()))
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
        for (auto &child : world_group.children)
        {
            renderImGuiWorldGroupMenu(child);
        }

        ImGui::TreePop();
    }
}

void Engine::renderImGui()
{
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("CG Engine");

        if (ImGui::TreeNode("World"))
        {
            if (ImGui::TreeNode("Window"))
            {
                ImGui::Text("Width: %d, Height: %d", m_world.GetWindow().width, m_world.GetWindow().height);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Camera"))
            {
                auto &camera = m_world.GetCamera();
                ImGui::DragFloat3("Position", &camera.position.x, 0.05f);
                ImGui::DragFloat3("Looking At", &camera.looking_at.x, 0.05f);
                ImGui::DragFloat3("Up", &camera.up.x, 0.05f);
                ImGui::DragFloat("FOV", &camera.fov, 0.05f, 1.0f, 179);
                ImGui::DragFloat("Near", &camera.near, 0.05f, 0, camera.far - 1);
                ImGui::DragFloat("Far", &camera.far, 0.05f, camera.near + 1, 10000);

                // TODO: Implement reset
                // Reset
                // if (ImGui::Button("Reset"))
                // {
                //     m_camera_pos = {5, 5, 5};
                //     m_camera_looking_at = {0, 0, 0};
                // }

                ImGui::TreePop();
            }

            renderImGuiWorldGroupMenu(m_world.GetParentWorldGroup());

            ImGui::TreePop();
        }

        if (ImGui::Checkbox("VSync", &settings.vsync))
        {
            SetVsync(settings.vsync);
        }

        if (ImGui::Checkbox("Wireframe", &settings.wireframe))
        {
            SetWireframe(settings.wireframe);
        }

        ImGui::Checkbox("Render Axis", &settings.render_axis);

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    ImGui::Render();
}

void Engine::postRenderImGui() { ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData()); }
