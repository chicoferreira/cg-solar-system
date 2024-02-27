#ifndef MODEL_H
#define MODEL_H
#include <istream>
#include <optional>
#include <utility>
#include <vector>

#include "Vec.h"

enum class ModelLoadFormat
{
    OBJ,
    _3D
};

class Model
{
    std::vector<Vec3f> m_vertex;

public:
    Model() = default;

    explicit Model(std::vector<Vec3f> vertex) : m_vertex(std::move(vertex)) {}
    void Render() const;

    std::vector<Vec3f> &GetVertex() { return m_vertex; }

    void LoadFromObjStream(std::istream &file);
    void LoadFrom3dFormatStream(std::istream &file);
};

std::optional<Model> LoadModelFromFile(const std::string &file_path, ModelLoadFormat format);

#endif // MODEL_H
