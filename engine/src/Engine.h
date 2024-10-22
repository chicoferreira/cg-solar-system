#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Frustum.h"
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

    struct EngineSettings
    {
        size_t mssa_samples = 8;
        bool mssa = true;
        bool vsync = true;
        bool wireframe = false;
        bool render_axis = true;
        bool cull_faces = true;
        bool lighting = true;
        bool render_transform_through_points_path = false;
        bool render_normals = false;
        bool render_light_models = false;
        bool render_aabb = false;
        bool frustum_culling = true;
        bool fullscreen = false;
        Color background_color = Color(0.0f, 0.0f, 0.0f, 1.0f);
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
        void SetFullscreen(bool fullscreen);
        void ToggleFullscreen();
        static void SetMssa(bool enable);
        void ProcessInput(float timestep);
        void Run();
        void Shutdown() const;
        world::World &getWorld() { return m_world; }

        void UpdateViewport();

    private:
        world::World m_world;
        input::Input m_input;

        std::vector<model::Model> m_models;
        std::vector<uint32_t> m_models_vertex_buffers;
        std::vector<uint32_t> m_models_index_buffers;
        std::vector<uint32_t> m_models_tex_coords_buffers;
        std::vector<uint32_t> m_models_normal_buffers;

        std::vector<model::Texture> m_textures;
        std::vector<uint32_t> m_texture_buffers;

        EngineSettings m_settings;

        EngineSimulationTime m_simulation_time;

        EngineSystemEnvironment m_system_environment;
        ImGuiIO *io = nullptr;

        GLFWwindow *m_window = nullptr;
        utils::OperatingSystem m_os = utils::OperatingSystem::UNKNOWN;

        size_t m_current_rendered_models_size = 0;
        size_t m_current_rendered_indexes_size = 0;

        void setupEnvironment();

        void initImGui();
        void shutdownImGui() const;
        void renderImGui();
        static void postRenderImGui();
        void renderAxis();
        void renderImGuiWorldGroupMenu(
            world::WorldGroup &world_group,
            size_t world_group_index,
            world::WorldGroup *parent_group = nullptr
        );
        Mat4f applyTransformMatrix(world::GroupTransform &transformations, float time);
        void renderCatmullRomCurves(world::transform::TranslationThroughPoints &translation) const;
        void renderGroup(world::WorldGroup &group, const Frustum &frustum, const Mat4f &current_transform);
        void renderModel(const world::GroupModel &model, size_t index_count) const;
        void renderModelNormals(model::Model &model) const;
        void renderLights();
        void renderLightModel(const world::lighting::Light &light) const;
        void renderGlobalAABB(AABB &aabb);

        bool loadWorld();
        bool loadModels();
        bool loadTextures();
        void uploadModelsToGPU();
        void destroyModels() const;
        void setupWorldLights();
        void uploadTexturesToGPU();
        Frustum getCurrentFrustum();
    };
} // namespace engine

#endif // ENGINE_H
