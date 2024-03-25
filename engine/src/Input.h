#ifndef INPUT_H
#define INPUT_H
#include <GLFW/glfw3.h>

namespace engine::input
{
    enum State
    {
        NOT_PRESSED,
        FIRST_PRESSED,
        HOLD,
        RELEASED
    };

    class Input
    {
        State m_key[GLFW_KEY_LAST + 1] = {NOT_PRESSED};

    public:
        void UpdateKey(GLFWwindow *window, const int key) { ProcessGlfwKey(key, glfwGetKey(window, key)); }

        void UpdateButton(GLFWwindow *window, const int key) { ProcessGlfwKey(key, glfwGetMouseButton(window, key)); }

        void ProcessGlfwKey(const int key, const int action)
        {
            if (action == GLFW_PRESS)
            {
                if (m_key[key] == NOT_PRESSED || m_key[key] == RELEASED)
                    m_key[key] = FIRST_PRESSED;
                else
                    m_key[key] = HOLD;
            }
            else if (action == GLFW_RELEASE)
            {
                if (m_key[key] == HOLD || m_key[key] == FIRST_PRESSED)
                    m_key[key] = RELEASED;
                else
                    m_key[key] = NOT_PRESSED;
            }
        }
        State GetState(const int key) const { return m_key[key]; }
        bool IsPressEvent(const int key) const { return m_key[key] == FIRST_PRESSED; }
        bool IsReleaseEvent(const int key) const { return m_key[key] == RELEASED; }
        bool IsHolding(const int key) const { return m_key[key] == FIRST_PRESSED || m_key[key] == HOLD; }
    };
} // namespace engine::input

#endif // INPUT_H
