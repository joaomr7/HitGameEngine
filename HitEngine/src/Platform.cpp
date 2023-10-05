#include "Platform/Platform.h"
#include "Platform/Window.h"

#include "Core/Assert.h"
#include "Core/Memory.h"

namespace hit
{
    Window* Platform::s_main_window = nullptr;
    ui16 Platform::s_window_count = 0;

    bool Platform::initialize()
    {
        // initialize glfw
        if(!glfwInit())
        {
            hit_error("Failed to initialize GLFW!");
            return false;
        }

        // set glfw api to none
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        return true;
    }

    void Platform::shutdown()
    {
        // delete main window
        if(s_main_window)
        {
            destroy_window(s_main_window);
            s_main_window = nullptr;
        }

        hit_warning_if(s_window_count > 0, "{} windows wasn't released!", s_window_count);

        // terminate glfw
        glfwTerminate();
    }

    bool Platform::execute()
    {
        // update glfw
        glfwPollEvents();

        return true;
    }

    void Platform::set_main_window(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");

        s_main_window = window;
    }

    Window* Platform::create_window(const WindowSpecification& specification)
    {
        s_window_count++;
        return Memory::allocate_initialized_memory<Window>(MemoryUsage::Platform, specification);
    }

    void Platform::destroy_window(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");

        s_window_count--;
        Memory::deallocate_initialized_memory(window);
    }

    bool Platform::is_window_open(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");
        return window->is_running();
    }

    void Platform::close_window(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");
        return window->close_window();
    }

    ui16 Platform::get_window_width(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");
        return window->get_width();
    }

    ui16 Platform::get_window_height(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");
        return window->get_height();
    }

    const std::string& Platform::get_window_title(Window* window)
    {
        hit_assert(window, "Window ptr is invalid!");
        return window->get_title();
    }

    const Window* Platform::get_main_window()
    {
        return s_main_window;
    }
}