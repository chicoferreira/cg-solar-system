#ifndef WORLD_H
#define WORLD_H

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Color.h"
#include "Mat.h"
#include "Vec.h"

namespace world
{
    struct Window
    {
        int width{}, height{};

        Window() = default;
        Window(int width, int height) : width(width), height(height) {}
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
        Camera(const Vec3f &position, const Vec3f &looking_at, const Vec3f &up, float fov, float near, float far) :
            position(position), looking_at(looking_at), up(up), fov(fov), near(near), far(far)
        {
        }

        bool ToggleFirstPersonMode() { return first_person_mode = !first_person_mode; }
    };

    namespace transform
    {
        struct Rotation
        {
            float angle_rads{0};
            Vec3f axis{0, 0, 0};

            Mat4f GetTransform(float) const { return Mat4fRotate(angle_rads, axis.x, axis.y, axis.z); }

            Rotation() = default;
            explicit Rotation(const float angle_rads, Vec3f axis) : angle_rads(angle_rads), axis(std::move(axis)) {}
        };

        struct RotationWithTime
        {
            float time_to_complete = {1};
            Vec3f axis{0, 1, 0};

            Mat4f GetTransform(float time) const
            {
                return Mat4fRotate(time * M_PI * 2 / time_to_complete, axis.x, axis.y, axis.z);
            }

            RotationWithTime() = default;
            explicit RotationWithTime(const float time_to_complete, Vec3f axis) :
                time_to_complete(time_to_complete), axis(std::move(axis))
            {
            }
        };

        struct Translation
        {
            Vec3f translation{0, 0, 0};

            Mat4f GetTransform(float) const { return Mat4fTranslate(translation.x, translation.y, translation.z); }

            Translation() = default;
            explicit Translation(Vec3f translation) : translation(std::move(translation)) {}
        };

        struct TranslationThroughPoints
        {
            float time_to_complete = {1};
            bool align_to_path;
            std::vector<Vec3f> points_to_follow;

            uint32_t render_path_gpu_buffer = 0; // the buffer associated with the data uploaded to the GPU
            bool render_path_dirty = true; // true if new data needs to be uploaded to the GPU (points change)
            bool render_path = true;
            Vec3f last_y_vector = {0, 1, 0};

            Mat4f GetTransform(float time)
            {
                if (points_to_follow.size() < 4)
                    return Mat4fIdentity;

                Vec3f position;
                Vec3f derivative;
                getCatmullRomPoint(time / time_to_complete, points_to_follow, position, derivative);

                Mat4f result = Mat4fTranslate(position.x, position.y, position.z);

                if (align_to_path)
                {
                    Vec3f x_vector = derivative.Normalize();
                    Vec3f z_vector = x_vector.Cross(last_y_vector).Normalize();
                    Vec3f y_vector = z_vector.Cross(x_vector).Normalize();

                    last_y_vector = y_vector;

                    result *=
                        {{{x_vector.x, y_vector.x, z_vector.x, 0},
                          {x_vector.y, y_vector.y, z_vector.y, 0},
                          {x_vector.z, y_vector.z, z_vector.z, 0},
                          {0, 0, 0, 1}}};
                }

                return result;
            }

            void updatePoints() { render_path_dirty = true; }

            TranslationThroughPoints() = default;

            explicit TranslationThroughPoints(const float time, bool align, std::vector<Vec3f> points) :
                time_to_complete(time), align_to_path(align), points_to_follow(std::move(points))
            {
            }
        };

        struct Scale
        {
            Vec3f scale{1, 1, 1};

            Mat4f GetTransform(float) const { return Mat4fScale(scale.x, scale.y, scale.z); }

            Scale() = default;
            explicit Scale(Vec3f scale) : scale(std::move(scale)) {}
        };

        using Transform = std::variant<Rotation, RotationWithTime, Translation, TranslationThroughPoints, Scale>;
    } // namespace transform

    class GroupTransform
    {
        std::vector<transform::Transform> m_transformations = {};

    public:
        void AddTransform(const transform::Transform &transform) { m_transformations.push_back(transform); }
        std::vector<transform::Transform> &GetTransformations() { return m_transformations; }
        void RemoveTransform(const size_t index) { m_transformations.erase(m_transformations.begin() + index); }
    };

    namespace lighting {

    }

    struct ModelMaterial
    {
        Color diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        Color ambient = {1.0f, 1.0f, 1.0f, 1.0f};
        Color specular = {0.0f, 0.0f, 0.0f, 1.0f};
        Color emmisive = {0.0f, 0.0f, 0.0f, 1.0f};
        float shininess = 0.0f;
    };

    struct GroupModel
    {
        size_t model_index = 0; // Index of the model in World::m_model_names
        ModelMaterial material = {};
    };

    struct WorldGroup
    {
        std::optional<std::string> name = {};
        std::vector<GroupModel> models = {};
        GroupTransform transformations = {};
        std::vector<WorldGroup> children = {};

        WorldGroup() = default;
        explicit WorldGroup(std::string name) : name(std::make_optional(name)) {}
    };

    class World
    {
        std::string m_file_path;
        Window m_window;
        Camera m_camera;
        Camera m_default_camera;
        WorldGroup m_parent_world_group;

        std::vector<std::string> m_model_names = {};

    public:
        const std::string &GetFilePath() const { return m_file_path; }
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
        void ClearModelNames() { m_model_names.clear(); }

        void ResetCamera() { m_camera = m_default_camera; }

        explicit World(std::string file_path) : m_file_path(std::move(file_path)) {}
        World(std::string file_path, std::string main_group_name) :
            m_file_path(std::move(file_path)), m_parent_world_group(WorldGroup(main_group_name))
        {
        }
    };
} // namespace world

#endif // WORLD_H
