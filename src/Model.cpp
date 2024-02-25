#include "Model.h"

#include <GLFW/glfw3.h>

void Model::Render() const
{
    glBegin(GL_TRIANGLES);

    for (const auto& pos : m_positions)
    {
        glVertex3f(pos.x, pos.y, pos.z);
    }

    glEnd();
}
