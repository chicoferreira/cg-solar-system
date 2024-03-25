#ifndef MODEL_H
#define MODEL_H
#include <istream>
#include <optional>
#include <vector>

#include "Vec.h"

namespace engine::model
{
    enum class ModelLoadFormat
    {
        OBJ,
        _3D
    };

    std::optional<ModelLoadFormat> GetModelLoadFormat(const std::string &file_path);

    class Model
    {
        std::string m_name;
        std::vector<Vec3f> m_vertex;

    public:
        explicit Model(std::string name) : m_name(std::move(name)) {}

        explicit Model(std::string name, std::vector<Vec3f> vertex) :
            m_name(std::move(name)), m_vertex(std::move(vertex))
        {
        }

        const std::string &GetName() const { return m_name; }
        std::vector<Vec3f> &GetVertex() { return m_vertex; }

        void LoadFromObjStream(std::istream &file);
        void LoadFrom3dFormatStream(std::istream &file);
    };

    std::optional<Model> LoadModelFromFile(const std::string &file_path);
    std::optional<Model> LoadModelFromFile(const std::string &file_path, ModelLoadFormat format);
} // namespace engine::model

#endif // MODEL_H
