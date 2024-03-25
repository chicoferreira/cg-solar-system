#include "Model.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "Utils.h"

namespace engine::model
{
    size_t parseFirstIndex(std::string_view string)
    {
        if (const size_t pos = string.find('/'); pos != std::string::npos)
        {
            return std::stoi(std::string(string.substr(0, pos)));
        }
        return std::stoi(std::string(string));
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

        std::vector<Vec3f> temp_vertex;

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
                temp_vertex.push_back(vertex);
            }
            else if (type == "f")
            {
                std::string vertex1, vertex2, vertex3;
                stream >> vertex1 >> vertex2 >> vertex3;

                size_t i1 = parseFirstIndex(vertex1) - 1;
                size_t i2 = parseFirstIndex(vertex2) - 1;
                size_t i3 = parseFirstIndex(vertex3) - 1;

                m_vertex.push_back(temp_vertex[i1]);
                m_vertex.push_back(temp_vertex[i2]);
                m_vertex.push_back(temp_vertex[i3]);
            }
        }
    }

    void Model::LoadFrom3dFormatStream(std::istream &file)
    {
        std::string line;
        std::getline(file, line);
        m_vertex.resize(std::stoi(line));

        for (auto &pos : m_vertex)
        {
            std::getline(file, line);
            std::istringstream iss(line);
            iss >> pos.x >> pos.y >> pos.z;
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
