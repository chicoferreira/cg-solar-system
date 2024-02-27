#include "Model.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void Model::Render() const
{
    glBegin(GL_TRIANGLES);

    for (const auto &[x, y, z] : m_vertex)
    {
        glVertex3f(x, y, z);
    }

    glEnd();
}

size_t parseFirstIndex(std::string_view string)
{
    if (const size_t pos = string.find('/'); pos != std::string::npos)
    {
        return std::stoi(std::string(string.substr(0, pos)));
    }
    return std::stoi(std::string(string));
}

void Model::LoadFromObj(std::istream &file)
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
void Model::LoadFrom3dFormat(std::istream &file)
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
