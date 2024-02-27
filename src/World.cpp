#include "World.h"

#include <iostream>

#include "tinyxml2.h"


#define EARLY_RETURN_R(condition, message, result)                                                                     \
    if (condition)                                                                                                     \
    {                                                                                                                  \
        std::cerr << message << std::endl;                                                                             \
        return result;                                                                                                 \
    }

#define EARLY_RETURN(condition, message) EARLY_RETURN_R(condition, message, false)

#define LOAD_VEC3F(element, vec)                                                                                       \
    EARLY_RETURN(                                                                                                      \
        element->QueryFloatAttribute("x", &vec.x) != tinyxml2::XML_SUCCESS,                                            \
        "World XML file is missing " #vec " position x attribute."                                                     \
    );                                                                                                                 \
    EARLY_RETURN(                                                                                                      \
        element->QueryFloatAttribute("y", &vec.y) != tinyxml2::XML_SUCCESS,                                            \
        "World XML file is missing " #vec " position y attribute."                                                     \
    );                                                                                                                 \
    EARLY_RETURN(                                                                                                      \
        element->QueryFloatAttribute("z", &vec.z) != tinyxml2::XML_SUCCESS,                                            \
        "World XML file is missing " #vec " position z attribute."                                                     \
    );

std::optional<WorldGroup> LoadWorldGroupFromXml(const tinyxml2::XMLElement *group_element, bool *success)
{
    WorldGroup group;

    const auto models_element = group_element->FirstChildElement("models");
    for (auto model_element = models_element->FirstChildElement("model"); model_element;
         model_element = model_element->NextSiblingElement("model"))
    {
        const auto file_path = model_element->Attribute("file");
        EARLY_RETURN_R(!file_path, "World XML model is missing the file attribute.", std::nullopt);

        const auto model = LoadModelFromFile(file_path, ModelLoadFormat::_3D); // TODO: Support other formats
        EARLY_RETURN_R(!model, "Failed to read model from file: '" << file_path << "'", std::nullopt);

        group.models.push_back(model.value());
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
    EARLY_RETURN(doc.LoadFile(file_path.c_str()) != tinyxml2::XML_SUCCESS, "World XML file not found or corrupt.");

    const auto world_element = doc.FirstChildElement("world");
    EARLY_RETURN(!world_element, "World XML file is missing the world element.");

    const auto window_element = world_element->FirstChildElement("window");
    EARLY_RETURN(!window_element, "World XML file is missing the window element.");

    EARLY_RETURN(
        window_element->QueryIntAttribute("width", &window.width) != tinyxml2::XML_SUCCESS,
        "World XML file is missing the window width attribute."
    );
    EARLY_RETURN(
        window_element->QueryIntAttribute("height", &window.height) != tinyxml2::XML_SUCCESS,
        "World XML file is missing the window height attribute."
    );

    const auto camera_element = world_element->FirstChildElement("camera");
    EARLY_RETURN(!camera_element, "World XML file is missing the camera element.");

    const auto camera_position_element = camera_element->FirstChildElement("position");
    EARLY_RETURN(!camera_position_element, "World XML file is missing the camera position element.");
    LOAD_VEC3F(camera_position_element, camera.position);

    const auto camera_look_at_element = camera_element->FirstChildElement("lookAt");
    EARLY_RETURN(!camera_look_at_element, "World XML file is missing the camera lookAt element.");
    LOAD_VEC3F(camera_look_at_element, camera.looking_at);

    const auto camera_up_element = camera_element->FirstChildElement("up");
    EARLY_RETURN(!camera_up_element, "World XML file is missing the camera up element.");
    LOAD_VEC3F(camera_up_element, camera.up);

    const auto camera_projection_element = camera_element->FirstChildElement("projection");
    EARLY_RETURN(!camera_projection_element, "World XML file is missing the camera projection element.");

    EARLY_RETURN(
        camera_projection_element->QueryFloatAttribute("fov", &camera.fov),
        "World XML file is missing camera projection fov attribute."
    );
    EARLY_RETURN(
        camera_projection_element->QueryFloatAttribute("near", &camera.near),
        "World XML file is missing camera projection near attribute."
    );
    EARLY_RETURN(
        camera_projection_element->QueryFloatAttribute("far", &camera.far),
        "World XML file is missing camera projection far attribute."
    );

    const auto parent_group_element = world_element->FirstChildElement("group");
    EARLY_RETURN(!parent_group_element, "World XML file is missing the parent group element.");

    bool succ = false;
    const auto parent_group = LoadWorldGroupFromXml(parent_group_element, &succ);

    if (!succ)
        return false;

    if (parent_group)
        parent_world_group = parent_group.value();

    return succ;
}
