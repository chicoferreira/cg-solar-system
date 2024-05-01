#include "Engine.h"

#include <cstdio>
#include <iostream>

#include "WorldSerde.h"

namespace engine
{
    void glfwErrorCallback(const int error, const char *description)
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

    void
    createModelBuffers(model::Model &model, uint32_t &vertex_buffer, uint32_t &normal_buffer, uint32_t &index_buffer)
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

        glGenBuffers(1, &normal_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
        glBufferData(
            GL_ARRAY_BUFFER, model.GetNormals().size() * sizeof(Vec3f), model.GetNormals().data(), GL_STATIC_DRAW
        );
    }

    void Engine::uploadModelsToGPU()
    {
        m_models_vertex_buffers.resize(m_models.size());
        m_models_normal_buffers.resize(m_models.size());
        m_models_index_buffers.resize(m_models.size());

        for (int i = 0; i < m_models.size(); ++i)
        {
            uint32_t vertex_buffer, normal_buffer, index_buffer;
            createModelBuffers(m_models[i], vertex_buffer, normal_buffer, index_buffer);

            m_models_vertex_buffers[i] = vertex_buffer;
            m_models_normal_buffers[i] = normal_buffer;
            m_models_index_buffers[i] = index_buffer;
        }
    }

    void Engine::destroyModels()
    {
        for (int i = 0; i < m_models.size(); ++i)
        {
            glDeleteBuffers(1, &m_models_vertex_buffers[i]);
            glDeleteBuffers(1, &m_models_normal_buffers[i]);
            glDeleteBuffers(1, &m_models_index_buffers[i]);
        }
    }

    bool Engine::Init()
    {
        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit())
            return false;

        loadWorld();

        m_os = utils::getOS();

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

        if (m_os == utils::OperatingSystem::MACOS)
            m_settings.mssa = false;

        SetVsync(m_settings.vsync);
        SetMssa(m_settings.mssa);
        SetLighting(m_settings.lighting);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHT0);
        // Rescale normals when scaling the model
        glEnable(GL_RESCALE_NORMAL);
        SetCullFaces(m_settings.cull_faces);
        SetWireframe(m_settings.wireframe);

#ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
#endif

        initImGui();

        setupEnvironment();

        auto result = loadModels();
        if (!result)
            return false;

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        float dark[4] = {0.2, 0.2, 0.2, 1.0};
        float white[4] = {1.0, 1.0, 1.0, 1.0};
        // light colors
        glLightfv(GL_LIGHT0, GL_AMBIENT, dark);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
        glLightfv(GL_LIGHT0, GL_SPECULAR, white);

        // To allow for ambient colors to be reproduced without having to activate the ambient component for all lights,
        // the following code should be added to the initialization:
        float amb[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

        uploadModelsToGPU();

        return true;
    }

    void Engine::renderAxis()
    {
        StartSectionDisableLighting();
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
        EndSectionDisableLighting();
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

    void Engine::renderModelNormals(model::Model &model)
    {
        StartSectionDisableLighting();
        glColor3f(0.0f, 0.5f, 1.0f);
        for (int i = 0; i < model.GetVertex().size(); ++i)
        {
            Vec3f vertex = model.GetVertex()[i];
            Vec3f normal = model.GetNormals()[i];
            glBegin(GL_LINES);
            glVertex3f(vertex.x, vertex.y, vertex.z);
            glVertex3f(vertex.x + normal.x, vertex.y + normal.y, vertex.z + normal.z);
            glEnd();
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        EndSectionDisableLighting();
    }

    void Engine::renderModel(world::GroupModel &model, size_t index_count)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, &model.material.ambient.r);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, &model.material.diffuse.r);
        glMaterialfv(GL_FRONT, GL_SPECULAR, &model.material.specular.r);
        glMaterialfv(GL_FRONT, GL_EMISSION, &model.material.emmisive.r);
        glMaterialf(GL_FRONT, GL_SHININESS, model.material.shininess);

        glBindBuffer(GL_ARRAY_BUFFER, m_models_normal_buffers[model.model_index]);
        glNormalPointer(GL_FLOAT, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, m_models_vertex_buffers[model.model_index]);
        glVertexPointer(3, GL_FLOAT, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_models_index_buffers[model.model_index]);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    }

    void Engine::renderGroup(world::WorldGroup &group)
    {
        glPushMatrix();

        renderTransformations(group.transformations, m_simulation_time.m_current_time);

        for (auto &group_model : group.models)
        {
            auto &model = m_models[group_model.model_index];
            auto model_index_size = model.GetIndexes().size();
            renderModel(group_model, model_index_size);
            if (m_settings.render_normals)
                renderModelNormals(model);

            m_current_rendered_models_size += 1;
            m_current_rendered_indexes_size += model_index_size;
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
        if (!m_settings.render_transform_through_points_path || !translation.render_path ||
            translation.points_to_follow.size() < 4)
            return;

        if (translation.render_path_gpu_buffer == 0)
            glGenBuffers(1, &translation.render_path_gpu_buffer);

        if (translation.render_path_dirty)
        {
            const size_t NUM_SEGMENTS = 100;

            std::vector<Vec3f> vertex;
            for (int i = 0; i < NUM_SEGMENTS; ++i)
            {
                const float time = static_cast<float>(i) / static_cast<float>(NUM_SEGMENTS);
                Vec3f position, derivative;
                getCatmullRomPoint(time, translation.points_to_follow, position, derivative);
                vertex.push_back(position);
            }

            glBindBuffer(GL_ARRAY_BUFFER, translation.render_path_gpu_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * vertex.size(), vertex.data(), GL_STATIC_DRAW);
            translation.render_path_dirty = false;
        }

        StartSectionDisableLighting();
        glColor3f(0.2f, 0.2f, 1.0f);
        glBindBuffer(GL_ARRAY_BUFFER, translation.render_path_gpu_buffer);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glDrawArrays(GL_LINE_LOOP, 0, 100);
        EndSectionDisableLighting();
    }

    void Engine::renderLightModel(const world::lighting::Light &light)
    {
        StartSectionDisableLighting();
        if (std::holds_alternative<world::lighting::DirectionalLight>(light))
        {
            const auto &directional_light = std::get<world::lighting::DirectionalLight>(light);
            Vec3f dir = directional_light.dir;

            glColor3f(1.0f, 1.0f, 0.5f);
            glBegin(GL_LINES);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(dir.x * 1000, dir.y * 1000, dir.z * 1000);
            glColor3f(1.0f, 1.0f, 1.0f);
            glEnd();
        }
        else if (std::holds_alternative<world::lighting::PointLight>(light))
        {
            const auto &point_light = std::get<world::lighting::PointLight>(light);
            Vec3f pos = point_light.pos;

            glPointSize(10.0f);
            glColor3f(1.0f, 1.0f, 0.5f);
            glBegin(GL_POINTS);
            glVertex3f(pos.x, pos.y, pos.z);
            glColor3f(1.0f, 1.0f, 1.0f);
            glEnd();
        }
        else if (std::holds_alternative<world::lighting::Spotlight>(light))
        {
            const auto &spot_light = std::get<world::lighting::Spotlight>(light);
            Vec3f pos = spot_light.pos;
            Vec3f dir = spot_light.dir;

            glPointSize(10.0f);
            glColor3f(1.0f, 1.0f, 0.5f);
            glBegin(GL_POINTS);
            glVertex3f(pos.x, pos.y, pos.z);
            glEnd();
            glBegin(GL_LINES);
            glVertex3f(pos.x, pos.y, pos.z);
            glVertex3f(pos.x + dir.x, pos.y + dir.y, pos.z + dir.z);
            glEnd();
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        EndSectionDisableLighting();
    }

    void Engine::renderLights()
    {
        const auto &lights = m_world.getLights();
        for (int i = 0; i < lights.size(); ++i)
        {
            const auto &light = lights[i];
            if (std::holds_alternative<world::lighting::DirectionalLight>(light))
            {
                const auto &directional_light = std::get<world::lighting::DirectionalLight>(light);
                Vec4f dir = directional_light.dir.ToVec4f(0.0f);

                glLightfv(GL_LIGHT0, GL_POSITION, &dir.x);
            }
            else if (std::holds_alternative<world::lighting::PointLight>(light))
            {
                const auto &point_light = std::get<world::lighting::PointLight>(light);
                Vec4f pos = point_light.pos.ToVec4f(1.0f);

                glLightfv(GL_LIGHT0, GL_POSITION, &pos.x);
            }
            else if (std::holds_alternative<world::lighting::Spotlight>(light))
            {
                const auto &spot_light = std::get<world::lighting::Spotlight>(light);
                Vec4f pos = spot_light.pos.ToVec4f(1.0f);
                Vec4f dir = spot_light.dir.ToVec4f(0.0f);

                glLightfv(GL_LIGHT0, GL_POSITION, &pos.x);
                glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, &dir.x);
                glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spot_light.cutoff);
            }
            renderLightModel(light);
        }
    }

    void Engine::Render()
    {
        renderImGui();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCamera(m_world.GetCamera(), m_world.GetWindow());

        if (m_settings.render_axis)
            renderAxis();

        renderLights();
        m_current_rendered_models_size = 0;
        m_current_rendered_indexes_size = 0;
        renderGroup(m_world.GetParentWorldGroup());

        postRenderImGui();
    }

    void Engine::SetVsync(const bool enable) { glfwSwapInterval(enable); }

    void Engine::SetWireframe(const bool enable) { glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL); }

    void Engine::SetCullFaces(const bool enable)
    {
        if (enable)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }

    void Engine::SetMssa(const bool enable)
    {
        if (enable)
            glEnable(GL_MULTISAMPLE);
        else
            glDisable(GL_MULTISAMPLE);
    }

    void Engine::SetLighting(const bool enable)
    {
        if (enable)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);
    }

    void Engine::StartSectionDisableLighting() const
    {
        if (m_settings.lighting)
            glDisable(GL_LIGHTING);
    }

    void Engine::EndSectionDisableLighting() const
    {
        if (m_settings.lighting)
            glEnable(GL_LIGHTING);
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

        Vec3f movement = Vec3f{};

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
        shutdownImGui();

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

} // namespace engine
