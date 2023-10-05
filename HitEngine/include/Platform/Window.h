#pragma once

#include "Core/Types.h"
#include "GLFW/glfw3.h"

#include <string>

namespace hit
{
    struct WindowSpecification
    {
        std::string window_title = "Hit Game Engine";
        ui16 window_width = 1280;
        ui16 window_height = 720;
    };

    class Window final
    {
    public:
        Window(const WindowSpecification& specification);
        ~Window();

        void close_window();
        bool is_running() const;

        inline const std::string& get_title() const { return m_specification.window_title; }

        inline ui16 get_width() const { return m_specification.window_width; }
        inline ui16 get_height() const { return m_specification.window_height; }

        inline const GLFWwindow* get_handle() const { return m_handle; }

    private:
        GLFWwindow* m_handle;
        WindowSpecification m_specification;
    };
}