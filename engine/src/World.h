#ifndef WORLD_H
#define WORLD_H

#include <variant>
#include <vector>

#include "Mat.h"
#include "Model.h"
#include "Vec.h"

namespace engine::world
{
    struct Window
    {
        int width{}, height{};

        Window() = default;
    };

    struct Camera
    {
        Vec3f position{}, looking_at{}, up{};
        float fov{}, near{}, far{};
        bool first_person_mode = false;

        Vec3f speed{};
        float scroll_speed = 0.0f;
        float max_speed_per_second = 10.0f;
        float acceleration_per_second = 100.0f;
        float friction_per_second = 20.0f;

        Camera() = default;

        void UpdateCameraRotation(float x_offset, float y_offset);
        void Tick(Vec3f input_movement, float scroll_input, float timestep);
        bool ToggleFirstPersonMode() { return first_person_mode = !first_person_mode; }
    };

    namespace transformation
    {
        struct Rotation
        {
            float angle_rads{};
            Vec3f axis{};

            Mat4f GetTransform() const { return Mat4fRotate(angle_rads, axis.x, axis.y, axis.z); }
            explicit Rotation(const float angle_rads, const Vec3f axis) : angle_rads(angle_rads), axis(axis) {}
        };

        struct Translation
        {
            Vec3f translation{};

            Mat4f GetTransform() const { return Mat4fTranslate(translation.x, translation.y, translation.z); }
            explicit Translation(const Vec3f translation) : translation(translation) {}
        };

        struct Scale
        {
            Vec3f m_scale;

            Mat4f GetTransform() const { return Mat4fScale(m_scale.x, m_scale.y, m_scale.z); }
            explicit Scale(const Vec3f scale) : m_scale(scale) {}
        };

        using Transform = std::variant<Rotation, Translation, Scale>;
    } // namespace transformation

    class GroupTransform
    {
        Mat4f m_final_transform = Mat4fIdentity;
        std::vector<transformation::Transform> m_transformations = {};

    public:
        Mat4f GetTransformMatrix() const { return m_final_transform; }
        void AddTransform(const transformation::Transform &transform) { m_transformations.push_back(transform); }
        void UpdateTransformMatrix();
        std::vector<transformation::Transform> &GetTransformations() { return m_transformations; }
        void RemoveTransform(const size_t index) { m_transformations.erase(m_transformations.begin() + index); }
    };

    struct WorldGroup
    {
        std::vector<model::Model> models = {};
        std::vector<WorldGroup> children = {};
        GroupTransform transformations = {};

        WorldGroup() = default;
    };

    class World
    {
        std::string m_name;
        Window m_window;
        Camera m_camera;
        Camera m_default_camera;
        WorldGroup m_parent_world_group;

    public:
        const std::string &GetName() const { return m_name; }
        Window &GetWindow() { return m_window; }
        Camera &GetCamera() { return m_camera; }
        void ResetCamera() { m_camera = m_default_camera; }
        WorldGroup &GetParentWorldGroup() { return m_parent_world_group; }

        bool LoadFromXml(const std::string &file_path);

        explicit World(std::string name) : m_name(std::move(name)) {}
    };
} // namespace engine::world

#endif // WORLD_H
