#include "Model.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utils.h"

namespace engine::model
{

    const std::vector<std::string> TEXTURE_PATHS_TO_SEARCH = {"assets/textures/", "./"};

    std::optional<Texture> LoadTextureFromFile(const std::string &file_path)
    {
        const auto path = utils::FindFile(TEXTURE_PATHS_TO_SEARCH, file_path);
        if (!path)
            return std::nullopt;


        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        uint8_t *data = stbi_load(path.value().string().c_str(), &width, &height, &channels, 4);

        if (!data)
        {
            if (stbi_failure_reason())
                std::cout << stbi_failure_reason();
            return std::nullopt;
        }

        const uint32_t image_width = static_cast<uint32_t>(width);
        const uint32_t image_height = static_cast<uint32_t>(height);

        return {Texture{file_path, image_width, image_height, std::unique_ptr<uint8_t>(data)}};
    }

    // Parse the first index of a face vertex
    // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 -> v1, v2, v3
    size_t parseFirstIndex(std::string_view string)
    {
        return std::stoi(std::string(string.substr(0, string.find('/'))));
    }

    std::optional<ModelLoadFormat> GetModelLoadFormat(const std::string &file_path)
    {
        if (file_path.ends_with(".obj"))
        {
            return std::make_optional(ModelLoadFormat::OBJ);
        }
        if (file_path.ends_with(".3d"))
        {
            return std::make_optional(ModelLoadFormat::_3D);
        }
        return std::nullopt;
    }

    void Model::LoadFromObjStream(std::istream &file)
    {
        std::string line;

        while (std::getline(file, line))
        {
            if (line[0] == '#')
            {
                continue;
            }

            std::istringstream stream(line);
            std::string type;
            stream >> type;

            if (type == "v")
            {
                Vec3f vertex;
                stream >> vertex.x >> vertex.y >> vertex.z;
                m_vertex.push_back(vertex);
                m_aabb.Extend(vertex);
            }
            else if (type == "vn")
            {
                Vec3f normal;
                stream >> normal.x >> normal.y >> normal.z;
                m_normals.push_back(normal);
            }
            else if (type == "vt")
            {
                Vec2f tex_coord;
                stream >> tex_coord.x >> tex_coord.y;
                m_tex_coords.push_back(tex_coord);
            }
            else if (type == "f")
            {
                std::string vertex1, vertex2, vertex3;
                stream >> vertex1 >> vertex2 >> vertex3;

                size_t i1 = parseFirstIndex(vertex1) - 1;
                size_t i2 = parseFirstIndex(vertex2) - 1;
                size_t i3 = parseFirstIndex(vertex3) - 1;

                m_indexes.push_back(i1);
                m_indexes.push_back(i2);
                m_indexes.push_back(i3);
            }
        }

        if (m_tex_coords.size() != m_vertex.size())
        {
            std::cerr << "Model " << m_name << " has no texture coordinates. Textures may not render properly."
                      << std::endl;
            m_tex_coords.resize(m_vertex.size(), Vec2f{0, 0});
        }
    }

    void Model::LoadFrom3dFormatStream(std::istream &file)
    {
        size_t vertex_size, indexes_size;
        file >> vertex_size >> indexes_size;

        m_vertex.resize(vertex_size, Vec3f{0, 0, 0});
        m_normals.resize(vertex_size, Vec3f{0, 0, 0});
        m_tex_coords.resize(vertex_size, Vec2f{0, 0});
        m_indexes.resize(indexes_size, 0);

        for (auto &pos : m_vertex)
        {
            file >> pos.x >> pos.y >> pos.z;
            m_aabb.Extend(pos);
        }

        for (auto &normal : m_normals)
        {
            file >> normal.x >> normal.y >> normal.z;
        }

        for (auto &tex_coord : m_tex_coords)
        {
            file >> tex_coord.x >> tex_coord.y;
        }

        for (auto &index : m_indexes)
        {
            file >> index;
        }
    }

    std::optional<Model> LoadModelFromFile(const std::string &file_path)
    {
        if (const auto model_load_format = GetModelLoadFormat(file_path))
            return LoadModelFromFile(file_path, model_load_format.value());

        return std::nullopt;
    }

    const std::vector<std::string> MODEL_PATHS_TO_SEARCH = {"assets/models/", "./"};

    std::optional<Model> LoadModelFromFile(const std::string &file_path, const ModelLoadFormat format)
    {
        const auto path = utils::FindFile(MODEL_PATHS_TO_SEARCH, file_path);
        if (!path)
        {
            return std::nullopt;
        }

        std::ifstream file(path.value());
        if (!file.is_open())
        {
            return std::nullopt;
        }

        Model model(path.value().string());

        switch (format)
        {
            case ModelLoadFormat::OBJ:
                model.LoadFromObjStream(file);
                break;
            case ModelLoadFormat::_3D:
                model.LoadFrom3dFormatStream(file);
                break;
        }

        return model;
    }
} // namespace engine::model
