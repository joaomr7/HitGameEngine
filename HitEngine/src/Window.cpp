#include "Platform/Window.h"

#include "Core/Assert.h"

namespace hit
{
    Window::Window(const WindowSpecification& specification, const EventCallback& callback)
    {
        hit_assert(callback, "Window callback is invalid!");

        m_data.title = specification.window_title;
        m_data.width = specification.window_width;
        m_data.height = specification.window_height;
        m_data.callback = callback;

        m_handle = glfwCreateWindow(
            (int)m_data.width,
            (int)m_data.height,
            m_data.title.c_str(),
            NULL, NULL);

        hit_assert(m_handle, "Failed to create GLFW window!");

        //setting up event system
        glfwSetWindowUserPointer(m_handle, &m_data);

        glfwSetKeyCallback(m_handle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyboardKeyPressedEvent event(key);
                    data.callback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyboardKeyReleasedEvent event(key);
                    data.callback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyboardKeyHoldEvent event(key);
                    data.callback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* window, int button, int action, int mods)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
            
            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.callback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.callback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_handle, [](GLFWwindow* window, double x_offset, double y_offset)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((f32)x_offset, (f32)y_offset);
            data.callback(event);
        });

        glfwSetCursorPosCallback(m_handle, [](GLFWwindow* window, double x_pos, double y_pos)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
            
            MouseMovedEvent event((f32)x_pos, (f32)y_pos);
            data.callback(event);
        });

        glfwSetWindowSizeCallback(m_handle, [](GLFWwindow* window, int width, int height)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.width = (ui16)width;
            data.height = (ui16)height;

            WindowResizeEvent event(data.width, data.height);
            data.callback(event);
        });

        glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* window)
        {
            auto& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            data.callback(event);
        });
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