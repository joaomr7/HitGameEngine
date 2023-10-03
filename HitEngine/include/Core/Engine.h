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

        std::string get_game_name() const;

    private:
        EngineData m_engine_data;
        ModulePipeline m_modules;
    };
}