#include "Core/Module.h"

#include "Core/Assert.h"
#include "Core/Log.h"

namespace hit
{
    ModulePipeline::~ModulePipeline()
    {
        shutdown_pipeline();
    }

    bool ModulePipeline::initialize_pipeline()
    {
        for(auto& module : m_modules)
        {
            module->m_engine = m_engine;
            if(!module->initialize())
            {
                return false;
            }
        }

        return true;
    }

    void ModulePipeline::shutdown_pipeline()
    {
        for(auto& module : m_modules)
        {
            module->shutdown();
        }

        m_engine = nullptr;
        m_modules.clear();
        m_modules_search_map.clear();
    }

    bool ModulePipeline::execute_modules()
    {
        for(auto& module : m_modules)
        {
            if(!module->execute())
            {
                return false;
            }
        }

        return true;
    }

    void ModulePipeline::set_engine(Engine* engine)
    {
        hit_assert(engine, "Setting an invalid Engine to modules pipeline system!");
        m_engine = engine;
    }

    bool ModulePipeline::add_module(const std::string& name, const Ref<Module>& module)
    {
        // modules name must be unique
        if(has_module(name))
        {
            hit_error("Module '{}', is already registerd!", name);
            return false;
        }

        m_modules.push_back(module);
        m_modules_search_map[name] = m_modules.size() - 1;

        return true;
    }

    bool ModulePipeline::has_module(const std::string& name) const
    {
        auto module_search = m_modules_search_map.find(name);
        return module_search != m_modules_search_map.end();
    }

    Ref<Module> ModulePipeline::get_module(const std::string& name) const
    {
        if(has_module(name))
        {
            return m_modules[m_modules_search_map.at(name)];
        }

        return nullptr;
    }
}