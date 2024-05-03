#include "Model.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "Utils.h"

namespace engine::model
{
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
            }
            else if (type == "vn")
            {
                Vec3f normal;
                stream >> normal.x >> normal.y >> normal.z;
                m_normals.push_back(normal);
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
    }

    void Model::LoadFrom3dFormatStream(std::istream &file)
    {
        size_t vertex_size, indexes_size;
        file >> vertex_size >> indexes_size;

        m_vertex.resize(vertex_size);
        m_normals.resize(vertex_size);
        m_indexes.resize(indexes_size);

        for (auto &pos : m_vertex)
        {
            file >> pos.x >> pos.y >> pos.z;
        }

        for (auto &normal : m_normals)
        {
            file >> normal.x >> normal.y >> normal.z;
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
