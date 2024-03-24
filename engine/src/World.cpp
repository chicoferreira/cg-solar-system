#include "World.h"

#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>

#include "tinyxml2.h"

constexpr auto sensitivity = 0.1f;
constexpr auto scroll_sensitivity = 10.0f;

void Camera::UpdateCameraRotation(float x_offset, float y_offset)
{
    float radius, alpha, beta;
    x_offset = degrees_to_radians(x_offset);
    y_offset = degrees_to_radians(y_offset);

    if (first_person_mode)
    {
        (looking_at - position).ToSpherical(radius, alpha, beta);
        alpha -= x_offset * sensitivity;
        beta += y_offset * sensitivity;
    }
    else
    {
        (position - looking_at).ToSpherical(radius, alpha, beta);

        alpha -= x_offset * sensitivity;
        beta -= y_offset * sensitivity;
    }

    if (beta > M_PI_2)
    {
        beta = M_PI_2 - 0.001f;
    }
    else if (beta < -M_PI_2)
    {
        beta = -M_PI_2 + 0.001f;
    }

    const auto after = Vec3fSpherical(radius, alpha, beta);
    if (first_person_mode)
    {
        looking_at = after + position;
    }
    else
    {
        position = after + looking_at;
    }
}

void Camera::Tick(const Vec3f input_movement, const float scroll_input, const float timestep)
{
    const Vec3f forward = (looking_at - position).Normalize();
    const Vec3f right = forward.Cross(up).Normalize();

    const Vec3f move_dir = (forward * input_movement.z + right * input_movement.x).Normalize() + up * input_movement.y;
    const auto acceleration = move_dir * acceleration_per_second * timestep;
    speed += acceleration;

    scroll_speed += scroll_input * scroll_sensitivity * acceleration_per_second * timestep;

    if (speed.Length() > max_speed_per_second)
    {
        speed = speed.Normalize() * max_speed_per_second;
    }

    const Vec3f final_speed = speed + forward * scroll_speed;

    position += final_speed * timestep;
    looking_at += speed * timestep;

    if (move_dir.x == 0 && move_dir.y == 0 && move_dir.z == 0)
    {
        speed -= speed * timestep * friction_per_second;
    }

    if (scroll_input == 0)
    {
        scroll_speed -= scroll_speed * timestep * friction_per_second;
    }
}

#define EARLY_RETURN_R(condition, message, result)                                                                     \
    if (condition)                                                                                                     \
    {                                                                                                                  \
        std::cerr << message << std::endl;                                                                             \
        return result;                                                                                                 \
    }

#define EARLY_RETURN_FALSE(condition, message) EARLY_RETURN_R(condition, message, false)

#define LOAD_ATTRIBUTE(element, attribute, variable, return_value)                                                     \
    EARLY_RETURN_R(                                                                                                    \
        element->QueryFloatAttribute(attribute, &variable) != tinyxml2::XML_SUCCESS,                                   \
        "World XML file is missing " #variable " " attribute " attribute.",                                            \
        return_value                                                                                                   \
    );

#define LOAD_VEC3F(element, vec, return_value)                                                                         \
    LOAD_ATTRIBUTE(element, "x", vec.x, return_value);                                                                 \
    LOAD_ATTRIBUTE(element, "y", vec.y, return_value);                                                                 \
    LOAD_ATTRIBUTE(element, "z", vec.z, return_value);

std::optional<WorldGroup> LoadWorldGroupFromXml(const tinyxml2::XMLElement *group_element, bool *success)
{
    WorldGroup group;

    if (const auto models_element = group_element->FirstChildElement("models"))
    {
        for (auto model_element = models_element->FirstChildElement("model"); model_element;
             model_element = model_element->NextSiblingElement("model"))
        {
            const auto file_path = model_element->Attribute("file");
            EARLY_RETURN_R(!file_path, "World XML model is missing the file attribute.", std::nullopt);

            const auto model = LoadModelFromFile(file_path);
            EARLY_RETURN_R(!model, "Failed to read model from file: '" << file_path << "'", std::nullopt);

            group.models.push_back(model.value());
        }
    }

    group.transform = Mat4fIdentity;

    if (const auto transform_elements = group_element->FirstChildElement("transform"))
    {
        for (auto transform_element = transform_elements->FirstChildElement(); transform_element;
             transform_element = transform_element->NextSiblingElement())
        {
            if (strcmp(transform_element->Name(), "translate") == 0)
            {
                Vec3f translate;
                LOAD_VEC3F(transform_element, translate, std::nullopt);

                group.transform *= Mat4fTranslate(translate.x, translate.y, translate.z);
            }
            else if (strcmp(transform_element->Name(), "scale") == 0)
            {
                Vec3f scale;
                LOAD_VEC3F(transform_element, scale, std::nullopt);
                group.transform *= Mat4fScale(scale.x, scale.y, scale.z);
            }
            else if (strcmp(transform_element->Name(), "rotate") == 0)
            {
                Vec4f rotate;
                LOAD_VEC3F(transform_element, rotate, std::nullopt);
                EARLY_RETURN_R(
                    transform_element->QueryFloatAttribute("angle", &rotate.w) != tinyxml2::XML_SUCCESS,
                    "Rotate missing angle attribute.",
                    std::nullopt
                );
                group.transform *= Mat4fRotate(degrees_to_radians(rotate.w), rotate.x, rotate.y, rotate.z);
            }
        }
        group.transform = group.transform.transpose();
    }

    for (auto child_group_element = group_element->FirstChildElement("group"); child_group_element;
         child_group_element = child_group_element->NextSiblingElement("group"))
    {
        bool succ = false;
        std::optional<WorldGroup> child_group = LoadWorldGroupFromXml(child_group_element, &succ);

        if (!succ)
            return std::nullopt;

        if (child_group)
            group.children.push_back(child_group.value());
    }

    *success = true;
    return std::make_optional(group);
}

bool World::LoadFromXml(const std::string &file_path)
{
    tinyxml2::XMLDocument doc;
    EARLY_RETURN_FALSE(
        doc.LoadFile(file_path.c_str()) != tinyxml2::XML_SUCCESS, "World XML file not found or corrupt."
    );

    const auto world_element = doc.FirstChildElement("world");
    EARLY_RETURN_FALSE(!world_element, "World XML file is missing the world element.");

    const auto window_element = world_element->FirstChildElement("window");
    EARLY_RETURN_FALSE(!window_element, "World XML file is missing the window element.");

    EARLY_RETURN_FALSE(
        window_element->QueryIntAttribute("width", &m_window.width) != tinyxml2::XML_SUCCESS,
        "World XML file is missing the window width attribute."
    );
    EARLY_RETURN_FALSE(
        window_element->QueryIntAttribute("height", &m_window.height) != tinyxml2::XML_SUCCESS,
        "World XML file is missing the window height attribute."
    );

    const auto camera_element = world_element->FirstChildElement("camera");
    EARLY_RETURN_FALSE(!camera_element, "World XML file is missing the camera element.");

    const auto camera_position_element = camera_element->FirstChildElement("position");
    EARLY_RETURN_FALSE(!camera_position_element, "World XML file is missing the camera position element.");
    LOAD_VEC3F(camera_position_element, m_camera.position, false);

    const auto camera_look_at_element = camera_element->FirstChildElement("lookAt");
    EARLY_RETURN_FALSE(!camera_look_at_element, "World XML file is missing the camera lookAt element.");
    LOAD_VEC3F(camera_look_at_element, m_camera.looking_at, false);

    const auto camera_up_element = camera_element->FirstChildElement("up");
    EARLY_RETURN_FALSE(!camera_up_element, "World XML file is missing the camera up element.");
    LOAD_VEC3F(camera_up_element, m_camera.up, false);

    const auto camera_projection_element = camera_element->FirstChildElement("projection");
    EARLY_RETURN_FALSE(!camera_projection_element, "World XML file is missing the camera projection element.");

    EARLY_RETURN_FALSE(
        camera_projection_element->QueryFloatAttribute("fov", &m_camera.fov),
        "World XML file is missing camera projection fov attribute."
    );
    EARLY_RETURN_FALSE(
        camera_projection_element->QueryFloatAttribute("near", &m_camera.near),
        "World XML file is missing camera projection near attribute."
    );
    EARLY_RETURN_FALSE(
        camera_projection_element->QueryFloatAttribute("far", &m_camera.far),
        "World XML file is missing camera projection far attribute."
    );

    const auto parent_group_element = world_element->FirstChildElement("group");
    EARLY_RETURN_FALSE(!parent_group_element, "World XML file is missing the parent group element.");

    bool succ = false;
    const auto parent_group = LoadWorldGroupFromXml(parent_group_element, &succ);

    if (!succ)
        return false;

    if (parent_group)
        m_parent_world_group = parent_group.value();

    m_default_camera = m_camera;

    return succ;
}
