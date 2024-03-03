#include "Engine.h"

#include <cstdio>
#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

void glfw_error_callback(const int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void glfwSetFramebufferSizeCallback(GLFWwindow *_, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

double last_scroll = 0;

void glfwScrollCallback(GLFWwindow *_window, const double _xoffset, const double yoffset) { last_scroll = yoffset; }

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
    glfwSetFramebufferSizeCallback(m_window, glfwSetFramebufferSizeCallback);
    glfwSetScrollCallback(m_window, glfwScrollCallback);
    SetVsync(settings.vsync);

    glEnable(GL_DEPTH_TEST);
    SetCullFaces(settings.cull_faces);

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
    glLoadIdentity();

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

void Engine::SetCullFaces(const bool enable)
{
    settings.cull_faces = enable;
    if (enable)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}
void Engine::ProcessInput()
{
    static double lastX = 0, lastY = 0;

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            m_world.GetCamera().ProcessInput(xpos - lastX, lastY - ypos, last_scroll);
        }
        else if (last_scroll != 0)
        {
            m_world.GetCamera().ProcessInput(0, 0, last_scroll);
        }
    }
    last_scroll = 0;

    lastX = xpos;
    lastY = ypos;
}

void Engine::Run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        glfwGetFramebufferSize(m_window, &m_world.GetWindow().width, &m_world.GetWindow().height);

        ProcessInput();

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
                Model &model = models[i];
                if (ImGui::TreeNode(&model, "Model #%d (%s)", i, model.GetName().c_str()))
                {
                    if (auto positions = model.GetVertex();
                        ImGui::TreeNode(&model.GetVertex(), "Vertices (%d)", positions.size()))
                    {
                        for (int p_index = 0; p_index < positions.size(); ++p_index)
                        {
                            const auto &[x, y, z] = positions[p_index];
                            ImGui::Text("%d. x: %.2f, y: %.2f, z: %.2f", p_index + 1, x, y, z);
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

        if (ImGui::TreeNode(&m_world, "World (%s)", m_world.GetName().c_str()))
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
                ImGui::DragFloat("Near", &camera.near, 0.05f, 0.05f, camera.far - 1);
                ImGui::DragFloat("Far", &camera.far, 0.05f, camera.near + 1, 10000);

                if (ImGui::Button("Reset"))
                {
                    m_world.ResetCamera();
                }

                ImGui::TreePop();
            }

            renderImGuiWorldGroupMenu(m_world.GetParentWorldGroup());

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Settings"))
        {
            if (ImGui::Checkbox("VSync", &settings.vsync))
            {
                SetVsync(settings.vsync);
            }

            if (ImGui::Checkbox("Cull Faces", &settings.cull_faces))
            {
                SetCullFaces(settings.cull_faces);
            }

            if (ImGui::Checkbox("Wireframe", &settings.wireframe))
            {
                SetWireframe(settings.wireframe);
            }

            ImGui::Checkbox("Render Axis", &settings.render_axis);
            ImGui::TreePop();
        }

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    ImGui::Render();
}

void Engine::postRenderImGui() { ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData()); }
