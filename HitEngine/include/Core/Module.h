#pragma once

#include "Core/Types.h"
#include "Utils/Ref.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace hit
{
    class Engine;

    class Module
    {
    public:
        virtual ~Module() = default;

    protected:
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual bool execute() = 0;

        const Engine* get_engine() { return m_engine; }

    private:
        Engine* m_engine;

        friend class ModulePipeline;
    };

    class ModulePipeline final
    {
    public:
        ModulePipeline() = default;
        ~ModulePipeline();

        // should be called after adding modules and setting engine
        bool initialize_pipeline();
        void shutdown_pipeline();

        bool execute_modules();

        void set_engine(Engine* engine);
        bool add_module(const std::string& name, const Ref<Module>& module);

        bool has_module(const std::string& name) const;
        // should not be called every frame, prefer get it in the creation step
        Ref<Module> get_module(const std::string& name) const;

    private:
        Engine* m_engine;
        std::vector<Ref<Module>> m_modules;
        std::unordered_map<std::string, ui64> m_modules_search_map;
    };
}