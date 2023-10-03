#include "Core/Engine.h"

namespace hit
{
    struct TestModule : public Module
    {
        bool initialize() override
        {
            Log::log_message(Log::LogLevel::Info, "Initializing engine Test Module!");
            Log::log_message(Log::LogLevel::Info, "Game name: {}.", get_engine()->get_game_name());

            return true;
        }

        void shutdown() override
        {
            Log::log_message(Log::LogLevel::Info, "Shutting down engine Test Module!");
        }

        bool execute() override
        {
            Log::log_message(Log::LogLevel::Info, "Running engine Test Module!");
            return true;
        }
    };

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
        m_modules.add_module("Test Module", create_ref<TestModule>());

        return m_modules.initialize_pipeline();
    }

    void Engine::shutdown()
    {
        m_modules.shutdown_pipeline();
        Memory::shutdown_memory_system();
        Log::shutdown_log_system();
    }

    void Engine::run()
    {
        m_modules.execute_modules();
    }

    bool Engine::has_module(const std::string& module_name) const
    {
        return m_modules.has_module(module_name);
    }

    Ref<Module> Engine::get_module(const std::string& module_name) const
    {
        return m_modules.get_module(module_name);
    }

    std::string Engine::get_game_name() const
    {
        return m_engine_data.game_name;
    }
}