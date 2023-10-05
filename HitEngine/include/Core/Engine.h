#pragma once

#include "Log.h"
#include "Types.h"
#include "Assert.h"
#include "Memory.h"
#include "Module.h"

#include <string>

namespace hit
{
    struct EngineData
    {
        std::string game_name;
        ui16 main_window_width;
        ui16 main_window_height;
    };

    class Engine final
    {
    public:
        Engine() = default;
        ~Engine() = default;

        bool initialize(EngineData data);
        void shutdown();

        void run();

        bool has_module(const std::string& module_name) const;
        Ref<Module> get_module(const std::string& module_name) const;

        inline const std::string& get_game_name() const { return m_engine_data.game_name; }
        inline ui16 get_window_width() const { return m_engine_data.main_window_width; }
        inline ui16 get_window_height() const { return m_engine_data.main_window_height; }

    private:
        EngineData m_engine_data;
        ModulePipeline m_modules;
    };
}