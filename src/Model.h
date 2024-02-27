#ifndef MODEL_H
#define MODEL_H
#include <istream>
#include <utility>
#include <vector>

#include "Vec.h"

class Model
{
    std::vector<Vec3f> m_vertex;

public:
    Model() = default;

    explicit Model(std::vector<Vec3f> vertex) : m_vertex(std::move(vertex)) {}
    void Render() const;

    std::vector<Vec3f> &GetVertex() { return m_vertex; }

    void LoadFromObj(std::istream &file);
    void LoadFrom3dFormat(std::istream &file);
};


#endif // MODEL_H
