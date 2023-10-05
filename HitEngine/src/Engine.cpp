#include "Core/Engine.h"
#include "Platform/Platform.h"
#include "Platform/Window.h"

namespace hit
{
    bool Engine::initialize(EngineData data)
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
        Window* main_window = nullptr;
        {
            WindowSpecification window_spec;
            window_spec.window_title = m_engine_data.game_name;
            window_spec.window_width = m_engine_data.main_window_width;
            window_spec.window_height = m_engine_data.main_window_height;

            auto platform = cast_ref<Platform>(m_modules.get_module("Platform"));
            main_window = platform->create_window(window_spec);

            hit_assert(main_window, "Failed to create main engine window!");

            platform->set_main_window(main_window);
        }

        while(main_window->is_running())
        {
            m_modules.execute_modules();
        }
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