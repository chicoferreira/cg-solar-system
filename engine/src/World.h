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
    Window window;
    Camera camera;
    Camera default_camera;
    WorldGroup parent_world_group;

public:
    Window &GetWindow() { return window; }
    Camera &GetCamera() { return camera; }
    void ResetCamera();
    WorldGroup &GetParentWorldGroup() { return parent_world_group; }

    bool LoadFromXml(const std::string &file_path);

    World() = default;
};

#endif // WORLD_H
