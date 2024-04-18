#include "Engine.h"

#include <cstdio>
#include <iostream>

#include "WorldSerde.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

namespace engine
{
    void glfw_error_callback(const int error, const char *description)
    {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    void glfwSetFramebufferSizeCallback(GLFWwindow *, const int width, const int height)
    {
        glViewport(0, 0, width, height);
    }

    float last_scroll = 0;

    void glfwScrollCallback(GLFWwindow *, const double, const double yoffset) { last_scroll = yoffset; }

    bool Engine::loadWorld()
    {
        if (!world::serde::LoadWorldFromXml(m_world.GetFilePath().c_str(), m_world))
        {
            std::cerr << "Failed to load world from xml" << std::endl;
            return false;
        }
        return true;
    }

    bool Engine::loadModels()
    {
        m_models.clear();
        for (const auto &model_name : m_world.GetModelNames())
        {
            std::optional<model::Model> model_optional = model::LoadModelFromFile(model_name);
            if (!model_optional.has_value())
            {
                std::cerr << "Failed to load model: " << model_name << std::endl;
                return false;
            }

            m_models.push_back(std::move(model_optional.value()));
        }
        return true;
    }

    void createModelBuffers(model::Model &model, uint32_t &vertex_buffer, uint32_t &index_buffer)
    {
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(
            GL_ARRAY_BUFFER, model.GetVertex().size() * sizeof(Vec3f), model.GetVertex().data(), GL_STATIC_DRAW
        );

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            model.GetIndexes().size() * sizeof(uint32_t),
            model.GetIndexes().data(),
            GL_STATIC_DRAW
        );
    }

    void Engine::uploadModelsToGPU()
    {
        m_models_vertex_buffers.resize(m_models.size());
        m_models_index_buffers.resize(m_models.size());

        for (int i = 0; i < m_models.size(); ++i)
        {
            uint32_t vertex_buffer, index_buffer;
            createModelBuffers(m_models[i], vertex_buffer, index_buffer);

            m_models_vertex_buffers[i] = vertex_buffer;
            m_models_index_buffers[i] = index_buffer;
        }
    }

    OperatingSystem getOS()
    {
#ifdef _WIN32
        return OperatingSystem::WINDOWS;
#elif __APPLE__
        return OperatingSystem::MACOS;
#elif __linux__
        return OperatingSystem::LINUX;
#else
        return OperatingSystem::UNKNOWN;
#endif
    }

    static const char *GetOSName(OperatingSystem os)
    {
        switch (os)
        {
            case OperatingSystem::WINDOWS:
                return "Windows";
            case OperatingSystem::LINUX:
                return "Linux";
            case OperatingSystem::MACOS:
                return "MacOS";
            case OperatingSystem::UNKNOWN:
                return "Unknown";
        }
    }

    void GLAPIENTRY MessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam
    )
    {
        fprintf(
            stderr,
            "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type,
            severity,
            message
        );
    }

    bool Engine::Init()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return false;

        loadWorld();

        m_os = getOS();

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

        if (m_os == OperatingSystem::MACOS)
            m_settings.mssa = false;

        SetVsync(m_settings.vsync);
        SetMssa(m_settings.mssa);

        glEnable(GL_DEPTH_TEST);
        SetCullFaces(m_settings.cull_faces);

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);

        initImGui();

        setupEnvironment();

        auto result = loadModels();
        if (!result)
            return false;

        glEnableClientState(GL_VERTEX_ARRAY);

        uploadModelsToGPU();

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

    void renderCamera(const world::Camera &camera, const world::Window &window)
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

    void Engine::renderModel(uint32_t model_index, size_t index_count)
    {
        //        glBindBuffer(GL_ARRAY_BUFFER, m_models_vertex_buffers[model_index]);
        //        std::cout << "Model index: " << model_index << std::endl;
        //        std::cout << "Index count: " << index_count << std::endl;
        //        std::cout << "Vertex buffer: " << m_models_vertex_buffers[model_index] << std::endl;
        //        std::cout << "Index buffer: " << m_models_index_buffers[model_index] << std::endl;
        //        glVertexPointer(3, GL_FLOAT, 0, 0);
        //        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_models_index_buffers[model_index]);
        //        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    }

    void Engine::renderGroup(world::WorldGroup &group)
    {
        glPushMatrix();

        renderTransformations(group.transformations, m_simulation_time.m_current_time);

        for (auto &model_index : group.models)
        {
            auto &model = m_models[model_index];
            renderModel(model_index, model.GetIndexes().size());
        }

        for (auto &child : group.children)
        {
            renderGroup(child);
        }

        glPopMatrix();
    }

    void Engine::renderTransformations(world::GroupTransform &transformations, float time)
    {
        for (auto &transformation : transformations.GetTransformations())
        {
            Mat4f matrix = std::visit([&](auto &&arg) { return arg.GetTransform(time); }, transformation);
            if (std::holds_alternative<world::transform::TranslationThroughPoints>(transformation))
            {
                auto &translation = std::get<world::transform::TranslationThroughPoints>(transformation);
                renderCatmullRomCurves(translation);
            }

            glMultMatrixf(*matrix.transpose().mat);
        }
    }

    void Engine::renderCatmullRomCurves(world::transform::TranslationThroughPoints &translation) const
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
        if (translation.render_path && translation.points_to_follow.size() >= 4)
        {
            const size_t NUM_SEGMENTS = 100;

            for (int i = 0; i < NUM_SEGMENTS; ++i)
            {
                const float time = static_cast<float>(i) / static_cast<float>(NUM_SEGMENTS);
                Vec3f position, derivative;
                getCatmullRomPoint(time, translation.points_to_follow, position, derivative);
                glVertex3f(position.x, position.y, position.z);
            }
        }
        glEnd();
        glColor3f(1.0f, 1.0f, 1.0f);
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

    constexpr auto sensitivity = 0.1f;
    constexpr auto scroll_sensitivity = 0.1f;

    void UpdateCameraRotation(world::Camera &camera, float x_offset, float y_offset)
    {
        x_offset = degrees_to_radians(x_offset) * sensitivity;
        y_offset = degrees_to_radians(y_offset) * sensitivity;

        float radius, alpha, beta;
        auto direction = camera.position - camera.looking_at;
        if (camera.first_person_mode)
        {
            direction = -direction;
            y_offset = -y_offset;
        }

        direction.ToSpherical(radius, alpha, beta);

        alpha -= x_offset;
        beta = std::clamp(beta - y_offset, -static_cast<float>(M_PI_2) + 0.001f, static_cast<float>(M_PI_2) - 0.001f);

        const auto new_direction = Vec3fSpherical(radius, alpha, beta);
        if (camera.first_person_mode)
            camera.looking_at = new_direction + camera.position;
        else
            camera.position = new_direction + camera.looking_at;
    }

    void TickCamera(world::Camera &camera, const Vec3f input_movement, const float scroll_input, const float timestep)
    {
        const Vec3f forward = (camera.looking_at - camera.position).Normalize();
        const Vec3f right = forward.Cross(camera.up).Normalize();

        const Vec3f move_dir =
            (forward * input_movement.z + right * input_movement.x).Normalize() + camera.up * input_movement.y;
        const auto acceleration = move_dir * camera.acceleration_per_second * timestep;
        camera.speed += acceleration;

        camera.scroll_speed += scroll_input * scroll_sensitivity * camera.acceleration_per_second;

        if (camera.speed.Length() > camera.max_speed_per_second)
        {
            camera.speed = camera.speed.Normalize() * camera.max_speed_per_second;
        }

        if ((camera.looking_at - camera.position).Length() > 1.0f || scroll_input < 0)
        {
            camera.position += forward * camera.scroll_speed * timestep;
        }

        camera.position += camera.speed * timestep;
        camera.looking_at += camera.speed * timestep;

        if (move_dir.x == 0 && move_dir.y == 0 && move_dir.z == 0)
        {
            camera.speed -= camera.speed * timestep * camera.friction_per_second;
        }

        if (scroll_input == 0)
        {
            camera.scroll_speed -= camera.scroll_speed * timestep * camera.friction_per_second;
        }
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
            m_input.UpdateKey(m_window, GLFW_KEY_R);

            if (m_input.IsReleaseEvent(GLFW_KEY_V))
            {
                m_world.GetCamera().ToggleFirstPersonMode();
            }

            if (m_input.IsPressEvent(GLFW_KEY_R))
            {
                m_world.ResetCamera();
            }

            if (m_world.GetCamera().first_person_mode)
            {
                movement.z += m_input.IsHolding(GLFW_KEY_W) - m_input.IsHolding(GLFW_KEY_S);
                movement.x += m_input.IsHolding(GLFW_KEY_D) - m_input.IsHolding(GLFW_KEY_A);
                movement.y += m_input.IsHolding(GLFW_KEY_SPACE) - m_input.IsHolding(GLFW_KEY_LEFT_CONTROL);
            }
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

            if (m_input.IsHolding(GLFW_MOUSE_BUTTON_LEFT))
            {
                const float xoffset = xpos - lastX;
                const float yoffset = lastY - ypos;
                UpdateCameraRotation(m_world.GetCamera(), xoffset, yoffset);
            }
        }
        else
        {
            last_scroll = 0;
        }

        TickCamera(m_world.GetCamera(), movement, last_scroll, timestep);

        last_scroll = 0;

        lastX = xpos;
        lastY = ypos;
    }

    void EngineSimulationTime::Update(float timestep)
    {
        if (!m_is_paused)
            m_current_time += timestep * m_current_simulation_speed_p_s;
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

            m_simulation_time.Update(timestep);

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
        io->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

        ImGui::GetStyle().WindowRounding = 5.0f;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL2_Init();
    }

    const char *getTransformationName(const world::transform::Transform &transform)
    {
        if (std::holds_alternative<world::transform::Rotation>(transform))
            return "Rotation";
        if (std::holds_alternative<world::transform::Translation>(transform))
            return "Translation";
        if (std::holds_alternative<world::transform::Scale>(transform))
            return "Scale";
        if (std::holds_alternative<world::transform::TranslationThroughPoints>(transform))
            return "Translation Through Points";
        if (std::holds_alternative<world::transform::RotationWithTime>(transform))
            return "Rotation with Time";
        return "Unknown";
    }

    void Engine::renderImGuiWorldGroupMenu(world::WorldGroup &world_group)
    {
        auto &model_indexes = world_group.models;
        auto world_group_name = world_group.name.has_value() ? world_group.name->c_str() : "unknown";
        if (ImGui::TreeNode(&world_group, "Group (%s)", world_group_name))
        {
            if (ImGui::TreeNodeEx(&model_indexes, ImGuiTreeNodeFlags_DefaultOpen, "Models (%zu)", model_indexes.size()))
            {
                for (int i = 0; i < model_indexes.size(); ++i)
                {
                    size_t &model_index = model_indexes[i];
                    ImGui::Text("Model #%lu (%s)", model_index, m_models[model_index].GetName().c_str());
                }
                ImGui::TreePop();
            }

            ImGui::BeginGroup();

            struct TransformDragDropPayload
            {
                world::transform::Transform *origin;
                world::WorldGroup *group;
                int group_index;
            };

            const auto num_transforms = world_group.transformations.GetTransformations().size();
            if (ImGui::TreeNode(&world_group.transformations, "Transformations (%zu)", num_transforms))
            {
                if (ImGui::Button("Add Transformation"))
                {
                    ImGui::OpenPopup("Add Transformation");
                }

                if (ImGui::BeginPopup("Add Transformation"))
                {
                    if (ImGui::MenuItem("Rotation"))
                    {
                        world_group.transformations.AddTransform(world::transform::Rotation());
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Rotation (with Time)"))
                    {
                        world_group.transformations.AddTransform(world::transform::RotationWithTime());
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Translation"))
                    {
                        world_group.transformations.AddTransform(world::transform::Translation());
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Translation (Through Points)"))
                    {
                        world_group.transformations.AddTransform(world::transform::TranslationThroughPoints());
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::MenuItem("Scale"))
                    {
                        world_group.transformations.AddTransform(world::transform::Scale());
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                for (int i = 0; i < world_group.transformations.GetTransformations().size(); ++i)
                {
                    if (i > 0 && i < world_group.transformations.GetTransformations().size())
                        ImGui::Separator();

                    auto &transform = world_group.transformations.GetTransformations()[i];

                    ImGui::BeginGroup();
                    ImGui::PushID(&transform);
                    ImGui::Text("%s", getTransformationName(transform));

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        TransformDragDropPayload transform_payload = {&transform, &world_group, i};
                        ImGui::SetDragDropPayload(
                            "TRANSFORMATION", &transform_payload, sizeof(TransformDragDropPayload)
                        );
                        ImGui::Text("%s", getTransformationName(transform));
                        ImGui::EndDragDropSource();
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        world_group.transformations.RemoveTransform(i);
                    }

                    if (std::holds_alternative<world::transform::Rotation>(transform))
                    {
                        auto &rotation = std::get<world::transform::Rotation>(transform);
                        float angle = radians_to_degrees(rotation.angle_rads);
                        ImGui::DragFloat3("Axis", &rotation.axis.x, 0.05f);

                        if (ImGui::DragFloat("Angle", &angle, 1, -360.0f, 360.0f))
                        {
                            rotation.angle_rads = degrees_to_radians(angle);
                        }
                    }
                    else if (std::holds_alternative<world::transform::RotationWithTime>(transform))
                    {
                        auto &rotation_with_time = std::get<world::transform::RotationWithTime>(transform);
                        ImGui::DragFloat3("Axis", &rotation_with_time.axis.x, 0.05f);
                        ImGui::DragFloat(
                            "Time to 360º", &rotation_with_time.time_to_complete, 0.01f, 0.0f, 0.0f, "%.3f s"
                        );
                    }
                    else if (std::holds_alternative<world::transform::Translation>(transform))
                    {
                        auto &translation = std::get<world::transform::Translation>(transform);
                        ImGui::DragFloat3("Coordinates", &translation.translation.x, 0.05f);
                    }
                    else if (std::holds_alternative<world::transform::TranslationThroughPoints>(transform))
                    {
                        auto &translation = std::get<world::transform::TranslationThroughPoints>(transform);
                        ImGui::DragFloat(
                            "Time to Complete", &translation.time_to_complete, 0.01f, 0.0f, 0.0f, "%.3f s"
                        );
                        ImGui::Checkbox("Align to Path", &translation.align_to_path);
                        ImGui::Checkbox("Render Path", &translation.render_path);

                        ImGui::Text("Points to Follow:");

                        if (translation.points_to_follow.size() < 4)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
                            ImGui::Text("This transformation needs at least 4 points to work.");
                            ImGui::PopStyleColor();
                        }

                        for (int point_index = 0; point_index < translation.points_to_follow.size(); ++point_index)
                        {
                            auto &point_to_follow = translation.points_to_follow[point_index];
                            ImGui::PushID(&point_to_follow);
                            std::string name = "Point " + std::to_string(point_index + 1);
                            ImGui::DragFloat3(name.c_str(), &point_to_follow.x, 0.05f);
                            ImGui::SameLine();
                            if (ImGui::SmallButton("Remove"))
                            {
                                translation.points_to_follow.erase(translation.points_to_follow.begin() + point_index);
                            }
                            ImGui::PopID();
                        }

                        if (ImGui::SmallButton("Add New Point"))
                        {
                            translation.points_to_follow.push_back({});
                        }
                    }
                    else if (std::holds_alternative<world::transform::Scale>(transform))
                    {
                        auto &scale = std::get<world::transform::Scale>(transform);
                        ImGui::DragFloat3("Axis", &scale.scale.x, 0.05f);
                    }
                    ImGui::EndGroup();
                    ImGui::PopID();

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("TRANSFORMATION"))
                        {
                            const auto transform_payload = *(const TransformDragDropPayload *)payload->Data;

                            if (transform_payload.group != &world_group || transform_payload.group_index != i)
                            {
                                auto &target_group_transformations =
                                    transform_payload.group->transformations.GetTransformations();

                                auto &origin = target_group_transformations[transform_payload.group_index];
                                auto &target = world_group.transformations.GetTransformations()[i];
                                std::swap(origin, target);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                ImGui::TreePop();
            }

            ImGui::EndGroup();

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("TRANSFORMATION"))
                {
                    const auto transform_payload = *(const TransformDragDropPayload *)payload->Data;

                    world_group.transformations.AddTransform(*transform_payload.origin);
                    transform_payload.group->transformations.RemoveTransform(transform_payload.group_index);
                }
                ImGui::EndDragDropTarget();
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

            auto flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
            if (ImGui::TreeNodeEx(&m_world, flags, "World (%s)", m_world.GetFilePath().c_str()))
            {
                ImGui::Text("Width:");
                ImGui::SameLine();
                ImGui::TextDisabled("%d", m_world.GetWindow().width);
                ImGui::SameLine();
                ImGui::Text("Height:");
                ImGui::SameLine();
                ImGui::TextDisabled("%d", m_world.GetWindow().height);

                if (ImGui::Button("Reload"))
                {
                    auto previous_window = m_world.GetWindow();
                    loadWorld();
                    m_world.GetWindow() = previous_window; // Window cannot be reloaded
                    loadModels();
                }

                if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Framed))
                {
                    auto &camera = m_world.GetCamera();
                    ImGui::DragFloat3("Position", &camera.position.x, 0.05f);
                    ImGui::DragFloat3("Looking At", &camera.looking_at.x, 0.05f);
                    ImGui::DragFloat3("Up", &camera.up.x, 0.05f);
                    ImGui::DragFloat("FOV", &camera.fov, 0.05f, 1.0f, 179);
                    ImGui::DragFloat("Near", &camera.near, 0.05f, 0.05f, camera.far - 1);
                    ImGui::DragFloat("Far", &camera.far, 0.05f, camera.near + 1, 10000);
                    ImGui::SeparatorText("Live Camera Settings");
                    ImGui::Checkbox("First Person Mode (V)", &camera.first_person_mode);
                    ImGui::DragFloat3("Speed", &camera.speed.x, 0.05f);
                    ImGui::DragFloat("Scroll Speed", &camera.scroll_speed, 0.05f);
                    ImGui::DragFloat("Max Speed", &camera.max_speed_per_second, 0.05f);
                    ImGui::DragFloat("Acceleration", &camera.acceleration_per_second, 0.05f);
                    ImGui::DragFloat("Friction", &camera.friction_per_second, 0.05f);

                    if (ImGui::Button("Reset (R)"))
                    {
                        m_world.ResetCamera();
                    }

                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Groups", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    renderImGuiWorldGroupMenu(m_world.GetParentWorldGroup());
                    ImGui::TreePop();
                }

                static float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

                if (ImGui::TreeNodeEx("Models", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (size_t i = 0; i < m_models.size(); ++i)
                    {
                        auto &model = m_models[i];
                        if (ImGui::TreeNode(&model, "Model #%zu (%s)", i, model.GetName().c_str()))
                        {
                            ImGui::Text("Triangle Count: %zu", model.GetVertex().size() / 3);
                            ImGui::Text("Vertex Count: %zu", model.GetVertex().size());

                            if (ImGui::TreeNodeEx("Vertices", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                ImGui::TreePop();
                                flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                    ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
                                ImVec2 outer_size = ImVec2(0, TEXT_BASE_HEIGHT * 10);
                                if (ImGui::BeginTable("vertex_table", 4, flags, outer_size))
                                {
                                    ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                                    ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_None);
                                    ImGui::TableSetupColumn("x", ImGuiTableColumnFlags_None);
                                    ImGui::TableSetupColumn("y", ImGuiTableColumnFlags_None);
                                    ImGui::TableSetupColumn("z", ImGuiTableColumnFlags_None);
                                    ImGui::TableHeadersRow();

                                    // Demonstrate using clipper for large vertical lists
                                    ImGuiListClipper clipper;
                                    clipper.Begin(model.GetVertex().size());
                                    while (clipper.Step())
                                    {
                                        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                                        {
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("%d", row);
                                            for (int column = 1; column < 4; column++)
                                            {
                                                ImGui::TableSetColumnIndex(column);
                                                ImGui::Text("%.3f", model.GetVertex()[row][column]);
                                            }
                                        }
                                    }
                                    ImGui::EndTable();
                                }
                            }

                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Simulation", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::Text("Current Time: %.2f", m_simulation_time.m_current_time);
                ImGui::Checkbox("Paused", &m_simulation_time.m_is_paused);
                ImGui::DragFloat("Simulation Speed", &m_simulation_time.m_current_simulation_speed_p_s, 0.05f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_Framed))
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

                if (m_os == OperatingSystem::MACOS)
                {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(Broken on MacOS)");
                }

                ImGui::SeparatorText("Environment Information");

                ImGui::BulletText("Detected Operating System: %s", GetOSName(m_os));
                ImGui::BulletText("MSSA Samples: %zu", m_settings.mssa_samples);
                ImGui::BulletText("GLEW Version: %s", m_system_environment.glew_version.c_str());
                ImGui::BulletText("GLFW Version: %s", m_system_environment.glfw_version.c_str());
                ImGui::BulletText("ImGui Version: %s", m_system_environment.imgui_version.c_str());
                ImGui::BulletText("OpenGL Version: %s", m_system_environment.opengl_version.c_str());
                ImGui::BulletText("GPU Renderer: %s", m_system_environment.gpu_renderer.c_str());
                ImGui::TreePop();
            }

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
            ImGui::End();
        }

        ImGui::Render();
    }

    void Engine::postRenderImGui() { ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData()); }

} // namespace engine
