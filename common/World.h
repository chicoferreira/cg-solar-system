#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <variant>
#include <vector>

#include "Mat.h"
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
        std::vector<transformation::Transform> &GetTransformations() { return m_transformations; }
        void RemoveTransform(const size_t index) { m_transformations.erase(m_transformations.begin() + index); }
        inline void UpdateTransformMatrix()
        {
            m_final_transform = Mat4fIdentity;
            for (auto &transformation : GetTransformations())
            {
                std::visit([&](auto &&arg) { m_final_transform *= arg.GetTransform(); }, transformation);
            }

            m_final_transform = m_final_transform.transpose();
        }
    };

    struct WorldGroup
    {
        std::vector<size_t> models = {}; // Indexes of the models in the world
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

        std::vector<std::string> m_model_names;

    public:
        const std::string &GetName() const { return m_name; }
        Window &GetWindow() { return m_window; }
        Camera &GetCamera() { return m_camera; }
        Camera &GetDefaultCamera() { return m_default_camera; }
        WorldGroup &GetParentWorldGroup() { return m_parent_world_group; }
        std::vector<std::string> &GetModelNames() { return m_model_names; }

        size_t AddModelName(const std::string &model_name)
        {
            if (std::find(m_model_names.begin(), m_model_names.end(), model_name) != m_model_names.end())
                return std::distance(
                    m_model_names.begin(), std::find(m_model_names.begin(), m_model_names.end(), model_name)
                );

            m_model_names.push_back(model_name);
            return m_model_names.size() - 1;
        }

        void ResetCamera() { m_camera = m_default_camera; }

        explicit World(std::string name) : m_name(std::move(name)) {}
    };
} // namespace engine::world

#endif // WORLD_H
