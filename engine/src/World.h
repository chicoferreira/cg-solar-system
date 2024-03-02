#ifndef WORLD_H
#define WORLD_H

#include <vector>
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

    Camera() = default;
};

struct WorldGroup
{
    std::vector<Model> models;
    std::vector<WorldGroup> children;

    WorldGroup(std::vector<Model> models, std::vector<WorldGroup> children) :
        models(std::move(models)), children(std::move(children))
    {
    }

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
