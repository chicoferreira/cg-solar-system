#ifndef MODEL_H
#define MODEL_H
#include <istream>
#include <utility>
#include <vector>

#include "Vec.h"

class Model
{
    std::vector<Vec3f> m_positions;

public:
    Model() = default;

    explicit Model(std::vector<Vec3f> positions) : m_positions(std::move(positions)) {}
    void Render() const;

    std::vector<Vec3f> &GetPositions() { return m_positions; }

    void WriteToFile(std::ofstream &file) const;
    void ReadFromFile(std::istream &file);
};


#endif // MODEL_H
