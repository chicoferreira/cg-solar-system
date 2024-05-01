#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Input.h"
#include "Model.h"
#include "Utils.h"
#include "World.h"

namespace engine
{
    class EngineSystemEnvironment
    {
    public:
        std::string glew_version = "unknown";
        std::string glfw_version = "unknown";
        std::string imgui_version = "unknown";
        std::string opengl_version = "unknown";
        std::string gpu_renderer = "unknown";

        EngineSystemEnvironment() = default;
    };

    class EngineSettings
    {
    public:
        EngineSettings(
            const size_t mssa_samples,
            const bool mssa,
            const bool vsync,
            const bool wireframe,
            const bool render_axis,
            const bool cull_faces,
            const bool render_transform_through_points_path,
            const bool lighting,
            const bool render_normals
        ) :
            mssa_samples(mssa_samples), mssa(mssa), vsync(vsync), wireframe(wireframe), render_axis(render_axis),
            cull_faces(cull_faces), render_transform_through_points_path(render_transform_through_points_path),
            lighting(lighting), render_normals(render_normals)
        {
        }

        size_t mssa_samples;
        bool mssa;
        bool vsync;
        bool wireframe;
        bool render_axis;
        bool cull_faces;
        bool render_transform_through_points_path;
        bool lighting;
        bool render_normals;
    };

    class EngineSimulationTime
    {
    public:
        void Update(float timestep);

        bool m_is_paused = false;
        float m_current_time = 0.0f; // seconds
        float m_current_simulation_speed_p_s = 1.0f; // 1.0f = 1 second per second
    };

    class Engine
    {
    public:
        explicit Engine(world::World world) : m_world(std::move(world)) {}

        bool Init();
        void Render();
        void SetVsync(bool enable);
        void SetWireframe(bool enable);
        void SetCullFaces(bool enable);
        void SetLighting(const bool enable);
        void StartSectionDisableLighting() const;
        void EndSectionDisableLighting() const;
        static void SetMssa(bool enable);
        void ProcessInput(float timestep);
        void Run();
        void Shutdown() const;

    private:
        world::World m_world;
        input::Input m_input;

        std::vector<model::Model> m_models;
        std::vector<uint32_t> m_models_vertex_buffers;
        std::vector<uint32_t> m_models_index_buffers;
        std::vector<uint32_t> m_models_normal_buffers;

        EngineSettings m_settings{
            8, // mssa_samples
            true, // mssa
            true, // vsync
            false, // wireframe
            true, // render_axis
            true, // cull_faces
            true, // render_transform_through_points_path
            true, // lighting
            false, // render normals
        };

        EngineSimulationTime m_simulation_time;

        EngineSystemEnvironment m_system_environment;
        ImGuiIO *io = nullptr;

        GLFWwindow *m_window = nullptr;
        utils::OperatingSystem m_os = utils::OperatingSystem::UNKNOWN;

        size_t m_current_rendered_models_size = 0;
        size_t m_current_rendered_triangles_size = 0;

        void setupEnvironment();

        void initImGui();
        void shutdownImGui() const;
        void renderImGui();
        static void postRenderImGui();
        void renderAxis();
        void renderImGuiWorldGroupMenu(world::WorldGroup &world_group);
        void renderTransformations(world::GroupTransform &transformations, float time);
        void renderCatmullRomCurves(world::transform::TranslationThroughPoints &translation) const;
        void renderGroup(world::WorldGroup &group);
        void renderModel(world::GroupModel &model, size_t index_count);
        void renderModelNormals(model::Model &model);
        void renderLights();
        void renderLightModel(const world::lighting::Light &light);

        bool loadWorld();
        bool loadModels();
        void uploadModelsToGPU();
        void destroyModels();
    };
} // namespace engine

#endif // ENGINE_H
