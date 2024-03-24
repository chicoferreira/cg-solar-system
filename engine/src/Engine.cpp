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

    glfwWindowHint(GLFW_SAMPLES, m_settings.mssa_samples);

    m_window = glfwCreateWindow(width, height, "CG", nullptr, nullptr);
    if (m_window == nullptr)
        return false;

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, glfwSetFramebufferSizeCallback);
    glfwSetScrollCallback(m_window, glfwScrollCallback);

    if (const GLenum err = glewInit(); err != GLEW_OK)
    {
        std::cerr << "Glew Error: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    SetVsync(m_settings.vsync);
    SetMssa(m_settings.mssa);

    glEnable(GL_DEPTH_TEST);
    SetCullFaces(m_settings.cull_faces);

    initImGui();

    setupEnvironment();

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
    glPushMatrix();

    const auto mat = group.transform.mat;
    glMultMatrixf(*mat);

    for (auto &model : group.models)
    {
        renderModel(model);
    }

    for (auto &child : group.children)
    {
        renderGroup(child);
    }

    glPopMatrix();
}

void Engine::Render()
{
    renderImGui();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCamera(m_world.GetCamera(), m_world.GetWindow());

    if (m_settings.render_axis)
        renderAxis();

    SetRenderWireframeMode(m_settings.wireframe);

    renderGroup(m_world.GetParentWorldGroup());

    postRenderImGui();
}

void Engine::SetVsync(const bool enable)
{
    m_settings.vsync = enable;
    glfwSwapInterval(enable);
}

void Engine::SetWireframe(const bool enable) { m_settings.wireframe = enable; }

void Engine::SetCullFaces(const bool enable)
{
    m_settings.cull_faces = enable;
    if (enable)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void Engine::SetMssa(const bool enable)
{
    if (enable)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}

void Engine::ProcessInput(const float timestep)
{
    static double lastX = 0, lastY = 0;

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    Vec3f movement = {};

    if (!io->WantCaptureKeyboard)
    {
        m_input.UpdateKey(m_window, GLFW_KEY_V);
        m_input.UpdateKey(m_window, GLFW_KEY_W);
        m_input.UpdateKey(m_window, GLFW_KEY_S);
        m_input.UpdateKey(m_window, GLFW_KEY_A);
        m_input.UpdateKey(m_window, GLFW_KEY_D);
        m_input.UpdateKey(m_window, GLFW_KEY_SPACE);
        m_input.UpdateKey(m_window, GLFW_KEY_LEFT_CONTROL);

        if (m_input.IsReleaseEvent(GLFW_KEY_V))
        {
            m_world.GetCamera().ToggleFirstPersonMode();
        }

        movement.z += m_input.IsHolding(GLFW_KEY_W) - m_input.IsHolding(GLFW_KEY_S);
        movement.x += m_input.IsHolding(GLFW_KEY_D) - m_input.IsHolding(GLFW_KEY_A);
        movement.y += m_input.IsHolding(GLFW_KEY_SPACE) - m_input.IsHolding(GLFW_KEY_LEFT_CONTROL);
    }

    if (!io->WantCaptureMouse)
    {
        m_input.UpdateButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
        m_input.UpdateButton(m_window, GLFW_MOUSE_BUTTON_RIGHT);

        if (m_input.IsPressEvent(GLFW_MOUSE_BUTTON_LEFT))
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (m_input.IsReleaseEvent(GLFW_MOUSE_BUTTON_LEFT))
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        if (!m_input.IsHolding(GLFW_MOUSE_BUTTON_LEFT))
        {
            xoffset = 0;
            yoffset = 0;
        }

        m_world.GetCamera().ProcessInput(xoffset, yoffset, last_scroll);
    }

    m_world.GetCamera().Tick(movement, timestep);

    last_scroll = 0;

    lastX = xpos;
    lastY = ypos;
}

void Engine::Run()
{
    float currentTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        glfwGetFramebufferSize(m_window, &m_world.GetWindow().width, &m_world.GetWindow().height);

        const float newTime = glfwGetTime();
        const float timestep = newTime - currentTime;
        ProcessInput(timestep);
        currentTime = newTime;

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

void Engine::setupEnvironment()
{
    m_system_environment.glew_version = std::string(reinterpret_cast<char const *>(glewGetString(GLEW_VERSION)));
    m_system_environment.glfw_version = glfwGetVersionString();
    m_system_environment.imgui_version = IMGUI_VERSION;
    m_system_environment.opengl_version = std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION)));
    m_system_environment.gpu_renderer = std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER)));
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
        if (ImGui::TreeNode("models", "Models (%zu)", models.size()))
        {
            for (int i = 0; i < models.size(); ++i)
            {
                Model &model = models[i];
                if (ImGui::TreeNode(&model, "Model #%d (%s)", i, model.GetName().c_str()))
                {
                    if (auto positions = model.GetVertex();
                        ImGui::TreeNode(&model.GetVertex(), "Vertices (%zu)", positions.size()))
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
                ImGui::Checkbox("First Person Mode", &camera.first_person_mode);
                if (camera.first_person_mode)
                {
                    ImGui::DragFloat3("Speed", &camera.speed.x, 0.05f);
                    ImGui::DragFloat("Max Speed", &camera.max_speed_per_second, 0.05f);
                    ImGui::DragFloat("Acceleration", &camera.acceleration_per_second, 0.05f);
                    ImGui::DragFloat("Friction", &camera.friction_per_second, 0.05f);
                }
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
            if (ImGui::Checkbox("VSync", &m_settings.vsync))
            {
                SetVsync(m_settings.vsync);
            }

            if (ImGui::Checkbox("Cull Faces", &m_settings.cull_faces))
            {
                SetCullFaces(m_settings.cull_faces);
            }

            if (ImGui::Checkbox("Wireframe", &m_settings.wireframe))
            {
                SetWireframe(m_settings.wireframe);
            }

            ImGui::Checkbox("Render Axis", &m_settings.render_axis);

            if (ImGui::Checkbox("MSSA", &m_settings.mssa))
            {
                SetMssa(m_settings.mssa);
            }

            if (ImGui::TreeNode("Environment"))
            {
                ImGui::Text("MSSA Samples: %zu", m_settings.mssa_samples);
                ImGui::Text("GLEW Version: %s", m_system_environment.glew_version.c_str());
                ImGui::Text("GLFW Version: %s", m_system_environment.glfw_version.c_str());
                ImGui::Text("ImGui Version: %s", m_system_environment.imgui_version.c_str());
                ImGui::Text("OpenGL Version: %s", m_system_environment.opengl_version.c_str());
                ImGui::Text("GPU Renderer: %s", m_system_environment.gpu_renderer.c_str());
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        ImGui::End();
    }

    ImGui::Render();
}

void Engine::postRenderImGui() { ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData()); }
