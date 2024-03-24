#ifndef WORLD_H
#define WORLD_H

#include <vector>

#include "Mat.h"
#include "Model.h"
#include "Vec.h"

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

    Vec3f speed;
    float max_speed_per_second = 10.0f;
    float acceleration_per_second = 100.0f;
    float friction_per_second = 10.0f;

    Camera() = default;

    void ProcessInput(float x_offset, float y_offset, float scroll_offset);
    void Tick(Vec3f input_movement, float timestep);
    bool ToggleFirstPersonMode() { return first_person_mode = !first_person_mode; }
};

struct WorldGroup
{
    std::vector<Model> models = {};
    std::vector<WorldGroup> children = {};
    Mat4f transform = Mat4fIdentity;

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

#endif // WORLD_H
