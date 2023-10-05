#include "Platform/Window.h"

#include "Core/Assert.h"

namespace hit
{
    Window::Window(const WindowSpecification& specification) : m_specification(specification)
    {
        m_handle = glfwCreateWindow(
            (int)m_specification.window_width,
            (int)m_specification.window_height,
            m_specification.window_title.c_str(),
            NULL, NULL);

        hit_assert(m_handle, "Failed to create GLFW window!");
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_handle);
    }

    void Window::close_window()
    {
        glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
    }

    bool Window::is_running() const
    {
        return !glfwWindowShouldClose(m_handle);
    }
}