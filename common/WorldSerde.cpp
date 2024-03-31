#include "WorldSerde.h"
#include <iostream>
#include <tinyxml2.h>

#include <cstdio>
#include <optional>

namespace engine::world::serde
{
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
    )

#define LOAD_VEC3F(element, vec, return_value)                                                                         \
    LOAD_ATTRIBUTE(element, "x", vec.x, return_value);                                                                 \
    LOAD_ATTRIBUTE(element, "y", vec.y, return_value);                                                                 \
    LOAD_ATTRIBUTE(element, "z", vec.z, return_value);

    std::optional<WorldGroup>
    LoadWorldGroupFromXml(World &world, const tinyxml2::XMLElement *group_element, bool *success)
    {
        WorldGroup group;

        if (const auto models_element = group_element->FirstChildElement("models"))
        {
            for (auto model_element = models_element->FirstChildElement("model"); model_element;
                 model_element = model_element->NextSiblingElement("model"))
            {
                const auto model_file_path = model_element->Attribute("file");
                EARLY_RETURN_R(!model_file_path, "World XML model is missing the file attribute.", std::nullopt)

                size_t model_index = world.AddModelName(model_file_path);
                group.models.push_back(model_index);
            }
        }

        group.transformations = {};

        if (const auto transform_elements = group_element->FirstChildElement("transform"))
        {
            for (auto transform_element = transform_elements->FirstChildElement(); transform_element;
                 transform_element = transform_element->NextSiblingElement())
            {
                if (strcmp(transform_element->Name(), "translate") == 0)
                {
                    Vec3f translate;
                    LOAD_VEC3F(transform_element, translate, std::nullopt)

                    group.transformations.AddTransform(transformation::Translation(translate));
                }
                else if (strcmp(transform_element->Name(), "scale") == 0)
                {
                    Vec3f scale;
                    LOAD_VEC3F(transform_element, scale, std::nullopt)
                    group.transformations.AddTransform(transformation::Scale(scale));
                }
                else if (strcmp(transform_element->Name(), "rotate") == 0)
                {
                    Vec3f axis;
                    float angle;
                    LOAD_VEC3F(transform_element, axis, std::nullopt)
                    EARLY_RETURN_R(
                        transform_element->QueryFloatAttribute("angle", &angle) != tinyxml2::XML_SUCCESS,
                        "Rotate missing angle attribute.",
                        std::nullopt
                    )
                    group.transformations.AddTransform(transformation::Rotation(degrees_to_radians(angle), axis));
                }
            }
            group.transformations.UpdateTransformMatrix();
        }

        for (auto child_group_element = group_element->FirstChildElement("group"); child_group_element;
             child_group_element = child_group_element->NextSiblingElement("group"))
        {
            bool succ = false;
            std::optional<WorldGroup> child_group = LoadWorldGroupFromXml(world, child_group_element, &succ);

            if (!succ)
                return std::nullopt;

            if (child_group)
                group.children.push_back(child_group.value());
        }

        *success = true;
        return std::make_optional(group);
    }

    bool LoadWorldFromXml(const char *file_path, engine::world::World &world)
    {
        tinyxml2::XMLDocument doc;
        EARLY_RETURN_FALSE(doc.LoadFile(file_path) != tinyxml2::XML_SUCCESS, "World XML file not found or corrupt.")

        const auto world_element = doc.FirstChildElement("world");
        EARLY_RETURN_FALSE(!world_element, "World XML file is missing the world element.")

        const auto window_element = world_element->FirstChildElement("window");
        EARLY_RETURN_FALSE(!window_element, "World XML file is missing the window element.")

        EARLY_RETURN_FALSE(
            window_element->QueryIntAttribute("width", &world.GetWindow().width) != tinyxml2::XML_SUCCESS,
            "World XML file is missing the window width attribute."
        )
        EARLY_RETURN_FALSE(
            window_element->QueryIntAttribute("height", &world.GetWindow().height) != tinyxml2::XML_SUCCESS,
            "World XML file is missing the window height attribute."
        )

        const auto camera_element = world_element->FirstChildElement("camera");
        EARLY_RETURN_FALSE(!camera_element, "World XML file is missing the camera element.")

        const auto camera_position_element = camera_element->FirstChildElement("position");
        EARLY_RETURN_FALSE(!camera_position_element, "World XML file is missing the camera position element.")
        LOAD_VEC3F(camera_position_element, world.GetCamera().position, false)

        const auto camera_look_at_element = camera_element->FirstChildElement("lookAt");
        EARLY_RETURN_FALSE(!camera_look_at_element, "World XML file is missing the camera lookAt element.")
        LOAD_VEC3F(camera_look_at_element, world.GetCamera().looking_at, false)

        const auto camera_up_element = camera_element->FirstChildElement("up");
        EARLY_RETURN_FALSE(!camera_up_element, "World XML file is missing the camera up element.")
        LOAD_VEC3F(camera_up_element, world.GetCamera().up, false)

        const auto camera_projection_element = camera_element->FirstChildElement("projection");
        EARLY_RETURN_FALSE(!camera_projection_element, "World XML file is missing the camera projection element.")

        EARLY_RETURN_FALSE(
            camera_projection_element->QueryFloatAttribute("fov", &world.GetCamera().fov),
            "World XML file is missing camera projection fov attribute."
        )
        EARLY_RETURN_FALSE(
            camera_projection_element->QueryFloatAttribute("near", &world.GetCamera().near),
            "World XML file is missing camera projection near attribute."
        )
        EARLY_RETURN_FALSE(
            camera_projection_element->QueryFloatAttribute("far", &world.GetCamera().far),
            "World XML file is missing camera projection far attribute."
        )

        const auto parent_group_element = world_element->FirstChildElement("group");
        EARLY_RETURN_FALSE(!parent_group_element, "World XML file is missing the parent group element.")

        bool succ = false;
        const auto parent_group = LoadWorldGroupFromXml(world, parent_group_element, &succ);

        if (!succ)
            return false;

        if (parent_group)
            world.GetParentWorldGroup() = parent_group.value();

        world.GetDefaultCamera() = world.GetCamera();

        return succ;
    }

} // namespace engine::world::serde
