#include "WorldSerde.h"
#include <iostream>
#include <tinyxml2.h>

#include <cstdio>
#include <optional>

namespace world::serde
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

#define LOAD_COLOR(material_element, material_field, color)                                                            \
    if (const auto element = material_element->FirstChildElement(material_field))                                      \
    {                                                                                                                  \
        element->QueryFloatAttribute("R", &color.r);                                                                   \
        element->QueryFloatAttribute("G", &color.g);                                                                   \
        element->QueryFloatAttribute("B", &color.b);                                                                   \
        color.r /= 255;                                                                                                \
        color.g /= 255;                                                                                                \
        color.b /= 255;                                                                                                \
    }

    std::optional<WorldGroup>
    LoadWorldGroupFromXml(World &world, const tinyxml2::XMLElement *group_element, bool *success)
    {
        WorldGroup group;

        const char *name = group_element->Attribute("name");
        if (name)
            group.name = name;

        if (const auto models_element = group_element->FirstChildElement("models"))
        {
            for (auto model_element = models_element->FirstChildElement("model"); model_element;
                 model_element = model_element->NextSiblingElement("model"))
            {
                const auto model_file_path = model_element->Attribute("file");
                EARLY_RETURN_R(!model_file_path, "World XML model is missing the file attribute.", std::nullopt)

                std::optional<size_t> texture_index = std::nullopt;
                if (const auto texture_element = model_element->FirstChildElement("texture"))
                {
                    const char *texture_file_path = texture_element->Attribute("file");
                    EARLY_RETURN_R(
                        !texture_file_path, "World XML model texture is missing the file attribute.", std::nullopt
                    )

                    texture_index = world.AddTextureName(texture_file_path);
                }

                ModelMaterial material;

                if (const auto color_element = model_element->FirstChildElement("color"))
                {
                    LOAD_COLOR(color_element, "diffuse", material.diffuse)
                    LOAD_COLOR(color_element, "ambient", material.ambient)
                    LOAD_COLOR(color_element, "specular", material.specular)
                    LOAD_COLOR(color_element, "emissive", material.emissive)
                    if (const auto shininess_element = color_element->FirstChildElement("shininess"))
                        shininess_element->QueryFloatAttribute("value", &material.shininess);
                }
                size_t model_index = world.AddModelName(model_file_path);
                group.models.push_back({model_index, texture_index, material});
            }
        }

        if (const auto transform_elements = group_element->FirstChildElement("transform"))
        {
            for (auto transform_element = transform_elements->FirstChildElement(); transform_element;
                 transform_element = transform_element->NextSiblingElement())
            {
                if (strcmp(transform_element->Name(), "translate") == 0)
                {
                    float time_to_complete;
                    if (transform_element->QueryFloatAttribute("time", &time_to_complete) == tinyxml2::XML_SUCCESS)
                    {
                        const char *align_string = transform_element->Attribute("align");

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif
                        bool align = align_string != nullptr && strcasecmp(align_string, "true") == 0;

                        std::vector<Vec3f> points_to_follow;
                        for (auto point_element = transform_element->FirstChildElement("point"); point_element;
                             point_element = point_element->NextSiblingElement("point"))
                        {
                            Vec3f point;
                            LOAD_VEC3F(point_element, point, std::nullopt)
                            points_to_follow.push_back(point);
                        }

                        group.transformations.AddTransform(
                            transform::TranslationThroughPoints(time_to_complete, align, points_to_follow)
                        );
                    }
                    else
                    {
                        Vec3f translate;
                        LOAD_VEC3F(transform_element, translate, std::nullopt)
                        group.transformations.AddTransform(transform::Translation(translate));
                    }
                }
                else if (strcmp(transform_element->Name(), "scale") == 0)
                {
                    Vec3f scale;
                    LOAD_VEC3F(transform_element, scale, std::nullopt)
                    group.transformations.AddTransform(transform::Scale(scale));
                }
                else if (strcmp(transform_element->Name(), "rotate") == 0)
                {
                    Vec3f axis;
                    float angle_or_time_to_full_rotation;
                    LOAD_VEC3F(transform_element, axis, std::nullopt)

                    if (transform_element->QueryFloatAttribute("angle", &angle_or_time_to_full_rotation) ==
                        tinyxml2::XML_SUCCESS)
                    {
                        group.transformations.AddTransform(
                            transform::Rotation(degrees_to_radians(angle_or_time_to_full_rotation), axis)
                        );
                    }
                    else if (transform_element->QueryFloatAttribute("time", &angle_or_time_to_full_rotation) ==
                             tinyxml2::XML_SUCCESS)
                    {
                        group.transformations.AddTransform(
                            transform::RotationWithTime(angle_or_time_to_full_rotation, axis)
                        );
                    }
                    else
                    {
                        std::cerr << "Rotation is missing rotate angle or time attribute." << std::endl;
                        return std::nullopt;
                    }
                }
            }
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

    bool LoadWorldFromXml(const char *file_path, world::World &world)
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

        world.getLights().clear();
        const auto parent_group_element = world_element->FirstChildElement("group");
        EARLY_RETURN_FALSE(!parent_group_element, "World XML file is missing the parent group element.")

        if (const auto lights_element = world_element->FirstChildElement("lights"))
        {
            for (auto light_element = lights_element->FirstChildElement(); light_element;
                 light_element = light_element->NextSiblingElement())
            {
                const char *type = light_element->Attribute("type");
                EARLY_RETURN_FALSE(!type, "World Light is missing type attribute")

                lighting::Light light;

                if (strcmp(type, "directional") == 0)
                {
                    lighting::DirectionalLight directional_light;
                    LOAD_ATTRIBUTE(light_element, "dirx", directional_light.dir.x, false)
                    LOAD_ATTRIBUTE(light_element, "diry", directional_light.dir.y, false)
                    LOAD_ATTRIBUTE(light_element, "dirz", directional_light.dir.z, false)
                    light = directional_light;
                }
                else if (strcmp(type, "point") == 0)
                {
                    lighting::PointLight point_light;
                    LOAD_ATTRIBUTE(light_element, "posx", point_light.pos.x, false)
                    LOAD_ATTRIBUTE(light_element, "posy", point_light.pos.y, false)
                    LOAD_ATTRIBUTE(light_element, "posz", point_light.pos.z, false)
                    light = point_light;
                }
                else if (strcmp(type, "spotlight") == 0 || strcmp(type, "spot") == 0)
                {
                    lighting::Spotlight spotlight;
                    LOAD_ATTRIBUTE(light_element, "dirx", spotlight.dir.x, false)
                    LOAD_ATTRIBUTE(light_element, "diry", spotlight.dir.y, false)
                    LOAD_ATTRIBUTE(light_element, "dirz", spotlight.dir.z, false)
                    LOAD_ATTRIBUTE(light_element, "posx", spotlight.pos.x, false)
                    LOAD_ATTRIBUTE(light_element, "posy", spotlight.pos.y, false)
                    LOAD_ATTRIBUTE(light_element, "posz", spotlight.pos.z, false)
                    LOAD_ATTRIBUTE(light_element, "cutoff", spotlight.cutoff, false)
                    light = spotlight;
                }
                else
                {
                    std::cerr << "Unknown light type '" << type << "' in World Lights" << std::endl;
                    return false;
                }

                world.getLights().push_back(light);
            }
        }
        else
        {
            world.SetDefaultLightingMode(false);
        }


        world.ClearModelNames();

        bool succ = false;
        const auto parent_group = LoadWorldGroupFromXml(world, parent_group_element, &succ);

        if (!succ)
            return false;

        if (parent_group.has_value())
            world.GetParentWorldGroup() = parent_group.value();

        world.GetDefaultCamera() = world.GetCamera();

        return succ;
    }

    void XmlSetVec3fAttribute(tinyxml2::XMLElement *element, const Vec3f &vec)
    {
        element->SetAttribute("x", vec.x);
        element->SetAttribute("y", vec.y);
        element->SetAttribute("z", vec.z);
    }

    void XmlSetColorAttribute(tinyxml2::XMLElement *element, const Color &color)
    {
        element->SetAttribute("R", color.r * 255);
        element->SetAttribute("G", color.g * 255);
        element->SetAttribute("B", color.b * 255);
    }

    void SaveWorldGroupToXml(
        tinyxml2::XMLDocument &doc,
        tinyxml2::XMLElement *parent_element,
        WorldGroup &group,
        World &world
    )
    {
        if (group.name != std::nullopt)
            parent_element->SetAttribute("name", group.name->c_str());

        tinyxml2::XMLElement *models_element = doc.NewElement("models");
        parent_element->InsertEndChild(models_element);

        for (GroupModel group_model : group.models)
        {
            tinyxml2::XMLElement *model_element = doc.NewElement("model");
            model_element->SetAttribute("file", world.GetModelNames()[group_model.model_index].c_str());

            if (group_model.texture_index != std::nullopt)
            {
                tinyxml2::XMLElement *texture_element = doc.NewElement("texture");
                texture_element->SetAttribute(
                    "file", world.GetTextureNames()[group_model.texture_index.value()].c_str()
                );
                model_element->InsertEndChild(texture_element);
            }

            ModelMaterial default_model_material;

            if (group_model.material != default_model_material)
            {
                tinyxml2::XMLElement *material_element = doc.NewElement("color");
                model_element->InsertEndChild(material_element);

                if (group_model.material.diffuse != default_model_material.diffuse)
                {
                    tinyxml2::XMLElement *diffuse_element = doc.NewElement("diffuse");
                    XmlSetColorAttribute(diffuse_element, group_model.material.diffuse);
                    material_element->InsertEndChild(diffuse_element);
                }

                if (group_model.material.ambient != default_model_material.ambient)
                {
                    tinyxml2::XMLElement *ambient_element = doc.NewElement("ambient");
                    XmlSetColorAttribute(ambient_element, group_model.material.ambient);
                    material_element->InsertEndChild(ambient_element);
                }

                if (group_model.material.specular != default_model_material.specular)
                {
                    tinyxml2::XMLElement *specular_element = doc.NewElement("specular");
                    XmlSetColorAttribute(specular_element, group_model.material.specular);
                    material_element->InsertEndChild(specular_element);
                }

                if (group_model.material.emissive != default_model_material.emissive)
                {
                    tinyxml2::XMLElement *emissive_element = doc.NewElement("emissive");
                    XmlSetColorAttribute(emissive_element, group_model.material.emissive);
                    material_element->InsertEndChild(emissive_element);
                }

                if (group_model.material.shininess != default_model_material.shininess)
                {
                    tinyxml2::XMLElement *shininess_element = doc.NewElement("shininess");
                    shininess_element->SetAttribute("value", group_model.material.shininess);
                    material_element->InsertEndChild(shininess_element);
                }
            }
            models_element->InsertEndChild(model_element);
        }

        if (!group.transformations.GetTransformations().empty())
        {
            tinyxml2::XMLElement *transform_element = doc.NewElement("transform");
            parent_element->InsertEndChild(transform_element);

            for (transform::Transform &transform : group.transformations.GetTransformations())
            {
                if (std::holds_alternative<transform::Translation>(transform))
                {
                    tinyxml2::XMLElement *translate_element = doc.NewElement("translate");
                    transform_element->InsertEndChild(translate_element);

                    const Vec3f &translate = std::get<transform::Translation>(transform).translation;
                    XmlSetVec3fAttribute(translate_element, translate);
                }
                else if (std::holds_alternative<transform::Scale>(transform))
                {
                    tinyxml2::XMLElement *scale_element = doc.NewElement("scale");
                    transform_element->InsertEndChild(scale_element);

                    const Vec3f &scale = std::get<transform::Scale>(transform).scale;
                    XmlSetVec3fAttribute(scale_element, scale);
                }
                else if (std::holds_alternative<transform::Rotation>(transform))
                {
                    tinyxml2::XMLElement *rotate_element = doc.NewElement("rotate");
                    transform_element->InsertEndChild(rotate_element);

                    const Vec3f &axis = std::get<transform::Rotation>(transform).axis;
                    XmlSetVec3fAttribute(rotate_element, axis);

                    float angle_rads = std::get<transform::Rotation>(transform).angle_rads;
                    float angle = radians_to_degrees(angle_rads);
                    rotate_element->SetAttribute("angle", angle);
                }
                else if (std::holds_alternative<transform::RotationWithTime>(transform))
                {
                    tinyxml2::XMLElement *rotate_element = doc.NewElement("rotate");
                    transform_element->InsertEndChild(rotate_element);

                    float time = std::get<transform::RotationWithTime>(transform).time_to_complete;
                    rotate_element->SetAttribute("time", time);

                    const Vec3f &axis = std::get<transform::RotationWithTime>(transform).axis;
                    XmlSetVec3fAttribute(rotate_element, axis);
                }
                else if (std::holds_alternative<transform::TranslationThroughPoints>(transform))
                {
                    const auto translation = std::get<transform::TranslationThroughPoints>(transform);
                    tinyxml2::XMLElement *translate_element = doc.NewElement("translate");
                    transform_element->InsertEndChild(translate_element);

                    translate_element->SetAttribute("time", translation.time_to_complete);
                    translate_element->SetAttribute("align", translation.align_to_path);

                    for (const auto &point : translation.points_to_follow)
                    {
                        tinyxml2::XMLElement *point_element = doc.NewElement("point");
                        XmlSetVec3fAttribute(point_element, point);
                        translate_element->InsertEndChild(point_element);
                    }
                }
            }
        }
        for (WorldGroup &child_group : group.children)
        {
            tinyxml2::XMLElement *group_element = doc.NewElement("group");
            parent_element->InsertEndChild(group_element);
            SaveWorldGroupToXml(doc, group_element, child_group, world);
        }
    }

    bool SaveWorldToXml(const char *filename, World &world)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement *world_element = doc.NewElement("world");
        doc.InsertFirstChild(world_element);

        tinyxml2::XMLElement *window_element = doc.NewElement("window");
        window_element->SetAttribute("width", world.GetWindow().width);
        window_element->SetAttribute("height", world.GetWindow().height);

        world_element->InsertEndChild(window_element);

        tinyxml2::XMLElement *camera_element = doc.NewElement("camera");
        world_element->InsertEndChild(camera_element);

        tinyxml2::XMLElement *camera_position_element = doc.NewElement("position");
        XmlSetVec3fAttribute(camera_position_element, world.GetCamera().position);

        camera_element->InsertEndChild(camera_position_element);

        tinyxml2::XMLElement *camera_look_at_element = doc.NewElement("lookAt");
        XmlSetVec3fAttribute(camera_look_at_element, world.GetCamera().looking_at);

        camera_element->InsertEndChild(camera_look_at_element);

        tinyxml2::XMLElement *camera_up_element = doc.NewElement("up");
        XmlSetVec3fAttribute(camera_up_element, world.GetCamera().up);

        camera_element->InsertEndChild(camera_up_element);

        tinyxml2::XMLElement *camera_projection_element = doc.NewElement("projection");
        camera_projection_element->SetAttribute("fov", world.GetCamera().fov);
        camera_projection_element->SetAttribute("near", world.GetCamera().near);
        camera_projection_element->SetAttribute("far", world.GetCamera().far);

        camera_element->InsertEndChild(camera_projection_element);

        tinyxml2::XMLElement *lights_element = doc.NewElement("lights");
        for (const lighting::Light &light : world.getLights())
        {
            tinyxml2::XMLElement *light_element = doc.NewElement("light");
            lights_element->InsertEndChild(light_element);

            if (std::holds_alternative<lighting::DirectionalLight>(light))
            {
                const lighting::DirectionalLight &directional_light = std::get<lighting::DirectionalLight>(light);
                light_element->SetAttribute("type", "directional");
                light_element->SetAttribute("dirx", directional_light.dir.x);
                light_element->SetAttribute("diry", directional_light.dir.y);
                light_element->SetAttribute("dirz", directional_light.dir.z);
            }
            else if (std::holds_alternative<lighting::PointLight>(light))
            {
                const lighting::PointLight &point_light = std::get<lighting::PointLight>(light);
                light_element->SetAttribute("type", "point");
                light_element->SetAttribute("posx", point_light.pos.x);
                light_element->SetAttribute("posy", point_light.pos.y);
                light_element->SetAttribute("posz", point_light.pos.z);
            }
            else if (std::holds_alternative<lighting::Spotlight>(light))
            {
                const lighting::Spotlight &spotlight = std::get<lighting::Spotlight>(light);
                light_element->SetAttribute("type", "spotlight");
                light_element->SetAttribute("dirx", spotlight.dir.x);
                light_element->SetAttribute("diry", spotlight.dir.y);
                light_element->SetAttribute("dirz", spotlight.dir.z);
                light_element->SetAttribute("posx", spotlight.pos.x);
                light_element->SetAttribute("posy", spotlight.pos.y);
                light_element->SetAttribute("posz", spotlight.pos.z);
                light_element->SetAttribute("cutoff", spotlight.cutoff);
            }
        }
        world_element->InsertEndChild(lights_element);

        tinyxml2::XMLElement *group_element = doc.NewElement("group");
        world_element->InsertEndChild(group_element);

        SaveWorldGroupToXml(doc, group_element, world.GetParentWorldGroup(), world);

        return doc.SaveFile(filename) == tinyxml2::XML_SUCCESS;
    }

} // namespace world::serde
