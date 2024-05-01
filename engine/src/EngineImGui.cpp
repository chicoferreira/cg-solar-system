#include "Engine.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace engine
{
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
#ifdef __APPLE__
        ImGui_ImplOpenGL3_Init("#version 120");
#else
        ImGui_ImplOpenGL3_Init();
#endif
    }

    void Engine::shutdownImGui() const
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
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
                    world::GroupModel &group_model = model_indexes[i];
                    auto model_index = group_model.model_index;
                    ImGui::Text("Model #%lu (%s)", model_index, m_models[model_index].GetName().c_str());

                    ImGui::ColorEdit4("Diffuse", &group_model.material.diffuse.r);
                    ImGui::ColorEdit4("Ambient", &group_model.material.ambient.r);
                    ImGui::ColorEdit4("Specular", &group_model.material.specular.r);
                    ImGui::ColorEdit4("Emmisive", &group_model.material.emmisive.r);
                    ImGui::DragFloat("Shininess", &group_model.material.shininess, 0.1f, 0.0f, 128.0f);
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

                        if (!m_settings.render_transform_through_points_path)
                        {
                            ImGui::SameLine();
                            ImGui::TextDisabled("(Disabled on Settings Page)");
                        }

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
                            if (ImGui::DragFloat3(name.c_str(), &point_to_follow.x, 0.05f))
                            {
                                translation.updatePoints();
                            }
                            ImGui::SameLine();
                            if (ImGui::SmallButton("Remove"))
                            {
                                translation.points_to_follow.erase(translation.points_to_follow.begin() + point_index);
                                translation.updatePoints();
                            }
                            ImGui::PopID();
                        }

                        if (ImGui::SmallButton("Add New Point"))
                        {
                            translation.points_to_follow.push_back(Vec3f{});
                            translation.updatePoints();
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
        ImGui_ImplOpenGL3_NewFrame();
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
                    destroyModels();
                    auto previous_window = m_world.GetWindow();
                    loadWorld();
                    m_world.GetWindow() = previous_window; // Window cannot be reloaded
                    loadModels();
                    uploadModelsToGPU();
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

                if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_Framed))
                {
                    for (auto &light : m_world.getLights())
                    {
                        ImGui::BeginGroup();
                        ImGui::PushID(&light);
                        if (std::holds_alternative<world::lighting::DirectionalLight>(light))
                        {
                            auto &directional_light = std::get<world::lighting::DirectionalLight>(light);
                            ImGui::Text("Directional Light");
                            ImGui::DragFloat3("Direction", &directional_light.dir.x, 0.05f);
                        }
                        else if (std::holds_alternative<world::lighting::PointLight>(light))
                        {
                            auto &point_light = std::get<world::lighting::PointLight>(light);
                            ImGui::Text("Point Light");
                            ImGui::DragFloat3("Position", &point_light.pos.x, 0.05f);
                        }
                        else if (std::holds_alternative<world::lighting::Spotlight>(light))
                        {
                            auto &spot_light = std::get<world::lighting::Spotlight>(light);
                            ImGui::Text("Spotlight");
                            ImGui::DragFloat3("Position", &spot_light.pos.x, 0.05f);
                            ImGui::DragFloat3("Direction", &spot_light.dir.x, 0.05f);
                            ImGui::DragFloat("Cutoff", &spot_light.cutoff, 0.05f);
                        }
                        ImGui::PopID();
                        ImGui::EndGroup();
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
                            ImGui::Text("Vertex Count: %zu", model.GetVertex().size());
                            ImGui::Text("Index Count: %zu", model.GetIndexes().size());
                            ImGui::Text("Triangle Count: %zu", model.GetIndexes().size() / 3);

                            flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
                            ImVec2 outer_size_vertex = ImVec2(
                                0, TEXT_BASE_HEIGHT * (std::min(model.GetVertex().size(), static_cast<size_t>(10)) + 1)
                            );

                            ImGui::Text("Vertex Table");
                            if (ImGui::BeginTable("vertex_table", 4, flags, outer_size_vertex))
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
                                            ImGui::Text("%.3f", model.GetVertex()[row][column - 1]);
                                        }
                                    }
                                }
                                ImGui::EndTable();
                            }

                            ImGui::Text("Triangle Table");
                            ImVec2 outer_size_index = ImVec2(
                                0,
                                TEXT_BASE_HEIGHT *
                                    (std::min(model.GetIndexes().size() / 3, static_cast<size_t>(10)) + 1)
                            );
                            if (ImGui::BeginTable("index_table", 4, flags, outer_size_index))
                            {
                                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                                ImGui::TableSetupColumn("Triangle No.", ImGuiTableColumnFlags_None);
                                ImGui::TableSetupColumn("Vertex 1", ImGuiTableColumnFlags_None);
                                ImGui::TableSetupColumn("Vertex 2", ImGuiTableColumnFlags_None);
                                ImGui::TableSetupColumn("Vertex 3", ImGuiTableColumnFlags_None);
                                ImGui::TableHeadersRow();

                                // Demonstrate using clipper for large vertical lists
                                ImGuiListClipper clipper;
                                clipper.Begin(model.GetIndexes().size() / 3);
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
                                            ImGui::Text("%d", model.GetIndexes()[row * 3 + (column - 1)]);
                                        }
                                    }
                                }
                                ImGui::EndTable();
                            }
                            ImGui::Text("Normals Table");
                            ImVec2 outer_size_normal = ImVec2(
                                0,
                                TEXT_BASE_HEIGHT *
                                    (std::min(model.GetNormals().size() / 3, static_cast<size_t>(10)) + 1)
                            );
                            if (ImGui::BeginTable("normal_table", 4, flags, outer_size_normal))
                            {
                                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                                ImGui::TableSetupColumn("Normal Index", ImGuiTableColumnFlags_None);
                                ImGui::TableSetupColumn("x", ImGuiTableColumnFlags_None);
                                ImGui::TableSetupColumn("y", ImGuiTableColumnFlags_None);
                                ImGui::TableSetupColumn("z", ImGuiTableColumnFlags_None);
                                ImGui::TableHeadersRow();

                                // Demonstrate using clipper for large vertical lists
                                ImGuiListClipper clipper;
                                clipper.Begin(model.GetNormals().size());
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
                                            ImGui::Text("%.3f", model.GetNormals()[row][column - 1]);
                                        }
                                    }
                                }
                                ImGui::EndTable();
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
                ImGui::Checkbox(
                    "Render Transform Through Points Path", &m_settings.render_transform_through_points_path
                );

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

                if (m_os == utils::OperatingSystem::MACOS)
                {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(Broken on MacOS)");
                }

                if (ImGui::Checkbox("Lighting", &m_settings.lighting))
                {
                    SetLighting(m_settings.lighting);
                }

                ImGui::Checkbox("Render Normals (May impact performance)", &m_settings.render_normals);

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

            ImGui::Text(
                "Rendering %zu models (%zu triangles)",
                m_current_rendered_models_size,
                m_current_rendered_triangles_size
            );
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
            ImGui::End();
        }

        ImGui::Render();
    }

    void Engine::postRenderImGui() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

} // namespace engine