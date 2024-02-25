#include "Model.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void Model::Render() const
{
    glBegin(GL_TRIANGLES);

    for (const auto &[x, y, z] : m_positions)
    {
        glVertex3f(x, y, z);
    }

    glEnd();
}

void Model::WriteToFile(std::ofstream &file) const
{
    file << m_positions.size() << std::endl;

    for (auto &[x, y, z] : m_positions)
    {
        file << x << ' ' << y << ' ' << z << '\n';
    }

    file.flush();
}

void Model::ReadFromFile(std::istream &file)
{
    std::string line;
    std::getline(file, line);
    m_positions.resize(std::stoi(line));

    for (auto &pos : m_positions)
    {
        std::getline(file, line);
        std::istringstream iss(line);
        iss >> pos.x >> pos.y >> pos.z;
    }
}