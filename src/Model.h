#ifndef MODEL_H
#define MODEL_H
#include <utility>
#include <vector>

#include "Vec.h"

class Model
{
    std::vector<Vec3f> m_positions;

public:
    explicit Model(std::vector<Vec3f> positions) : m_positions(std::move(positions))
    {
    }

    // Move constructor
    Model(Model&& other) noexcept
    {
        m_positions = std::move(other.m_positions);
    }

    void Render() const;
};


#endif //MODEL_H
