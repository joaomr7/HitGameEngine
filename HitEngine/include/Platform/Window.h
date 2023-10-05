#pragma once

#include "Core/Types.h"
#include "GLFW/glfw3.h"

#include "Event.h"

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
        Window(const WindowSpecification& specification, const EventCallback& callback);
        ~Window();

        void close_window();
        bool is_running() const;

        inline const std::string& get_title() const { return m_data.title; }

        inline ui16 get_width() const { return m_data.width; }
        inline ui16 get_height() const { return m_data.height; }

        inline const GLFWwindow* get_handle() const { return m_handle; }

    private:
        GLFWwindow* m_handle;

        struct WindowData
        {
            std::string title;
            ui16 width;
            ui16 height;

            EventCallback callback;
        } m_data;
    };
}