#include "Core/Engine.h"
#include "Platform/Platform.h"
#include "Platform/Window.h"

namespace hit
{
    bool Engine::initialize(EngineData data, const Ref<Module>& application_module)
    {
        if(!Log::initialize_log_system())
        {
            return false;
        }

        if(!Memory::initialize_memory_system())
        {
            hit_error("Failed to initialize engine memory system!");
            return false;
        }

        m_engine_data = data;

        m_modules.set_engine(this);
        m_modules.add_module("Platform", create_ref<Platform>());
        m_modules.add_module("Renderer", create_ref<Renderer>());

        if (application_module)
        {
            m_modules.add_module("Application", application_module);
        }

        m_invalid_window_size = false;

        return m_modules.initialize_pipeline();
    }

    void Engine::shutdown()
    {
        m_modules.shutdown_pipeline();

        if(!Memory::shutdown_memory_system())
        {
            hit_warning("Engine is leaking memory!");
        }

        Log::shutdown_log_system();
    }

    void Engine::run()
    {
        Window* main_window = (Window*)Platform::get_main_window();

        while(main_window->is_running()) [[likely]]
        {
            if(!m_modules.execute_modules() && !m_invalid_window_size) [[unlikely]]
            {
                hit_fatal("Engine main loop fails!");

                // force window to close
                main_window->close_window();
            }

            if(m_invalid_window_size)
            {
                Platform::wait_for_valid_window_size(main_window);
                m_invalid_window_size = false;
            }
        }
    }

    EventCallback Engine::get_event_callback()
    {
        return bind_event_function(Engine::handle_event, this);
    }

    void Engine::handle_event(Event& event)
    {
        EventHandler handler(event);

        handler.handle<WindowResizeEvent>(bind_event_function(Engine::handle_window_resize_event, this));
        handler.handle<WindowCloseEvent>(bind_event_function(Engine::handle_window_close_event, this));

        m_modules.handle_modules_events(event);
    }

    bool Engine::handle_window_resize_event(WindowResizeEvent& event)
    {
        m_engine_data.main_window_width = event.width;
        m_engine_data.main_window_height = event.height;

        auto renderer = cast_ref<Renderer>(m_modules.get_module("Renderer"));
        renderer->resize(m_engine_data.main_window_width, m_engine_data.main_window_height);

        m_invalid_window_size = event.width == 0 || event.height == 0;

        return false;
    }

    bool Engine::handle_window_close_event(WindowCloseEvent& event)
    {
        // just pass, at moment
        return false;
    }

    bool Engine::has_module(const std::string& module_name) const
    {
        return m_modules.has_module(module_name);
    }

    Ref<Module> Engine::get_module(const std::string& module_name) const
    {
        return m_modules.get_module(module_name);
    }
}