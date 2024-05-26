#include "Core/Memory.h"
#include "Core/Assert.h"
#include "Core/Log.h"

#include <unordered_map>
#include <string.h>

namespace hit::Memory
{
    struct MemorySystem
    {
        std::unordered_map<ui8*, Usage> usages;
#ifdef HIT_DEBUG
        std::unordered_map<Usage::Type, ui64> memory_used;
        ui64 total_memory_used;
#endif
    };

    static MemorySystem* s_memory_system = nullptr;

    // TODO: at the moment this system is unsafe
    bool initialize_memory_system()
    {
        s_memory_system = new MemorySystem();

#ifdef HIT_DEBUG
        s_memory_system->total_memory_used = 0;
        s_memory_system->memory_used[Usage::Any] = 0;
#endif

        return true;
    }

    bool shutdown_memory_system()
    {
        bool has_memory_leak = false;

        if (!s_memory_system->usages.empty())
        {
            hit_fatal("Memory leaks encountered!");

#ifdef HIT_DEBUG
            for(auto& [_memory, _usage] : s_memory_system->usages)
            {
                hit_trace("{}", _usage);
            }
#endif

            has_memory_leak = true;
        }

        delete s_memory_system;
        s_memory_system = nullptr;

        return !has_memory_leak;
    }

    ui8* allocate_memory(ui64 size, Usage::Type usage)
    {
        Usage memory_usage;
        memory_usage.usage = usage;
        memory_usage.size = size;

        ui8* out_memory = (ui8*)::operator new(size);
        hit_assert(out_memory, "Failed to allocate {} bytes!", size);

        memory_usage.memory = out_memory;

        s_memory_system->usages[out_memory] = memory_usage;

#ifdef HIT_DEBUG
        s_memory_system->memory_used[usage] += size;
        s_memory_system->total_memory_used += size;
#endif

        std::memset(out_memory, 0, size);

        return out_memory;
    }

    void deallocate_memory(ui8* memory)
    {
        if(!memory)
        {
            hit_warning("Attempting to deallocate an invalid memory!");
            return;
        }

        // find memory usage
        auto memory_usage_location = s_memory_system->usages.find(memory);
        hit_assert(
            memory_usage_location != s_memory_system->usages.end(), 
            "Attempting to deallocate a not registered memory!");

#ifdef HIT_DEBUG
        // update debug variables
        auto& usage = memory_usage_location->second;
        s_memory_system->total_memory_used -= usage.size;
        s_memory_system->memory_used[usage.usage] -= usage.size;
#endif

        s_memory_system->usages.erase(memory_usage_location);

        ::operator delete(memory);
    }

    ui8* reallocate_memory(ui8* memory, ui64 new_size)
    {
        if(!memory || !new_size)
        {
            hit_warning_if(!memory, "Attempting to reallocate an invalid memory!");
            hit_warning_if(!new_size, "Attempting to reallocate to a 0 size!");
            return nullptr;
        }

        // find memory usage
        auto memory_usage_location = s_memory_system->usages.find(memory);
        hit_assert(
            memory_usage_location != s_memory_system->usages.end(), 
            "Attempting to reallocate a not registered memory!");
        
        auto usage = memory_usage_location->second;
        
        if(new_size == usage.size)
        {
            hit_warning("Attempting to reallocate a memory to the same size!");
            return memory;
        }

#ifdef HIT_DEBUG
        // update debug variables
        if(new_size > usage.size)
        {
            s_memory_system->total_memory_used += new_size - usage.size;
            s_memory_system->memory_used[usage.usage] += new_size - usage.size;
        }
        else if(new_size < usage.size)
        {
            s_memory_system->total_memory_used -= usage.size - new_size;
            s_memory_system->memory_used[usage.usage] -= usage.size - new_size;
        }
#endif

        ui8* out_memory = (ui8*)std::realloc(memory, new_size);
        if(!out_memory)
        {
            hit_error("Failed to reallocate memory {}.", (void*)memory);
            return nullptr;
        }

        s_memory_system->usages.erase(memory_usage_location);
        s_memory_system->usages[out_memory] = { usage.usage, new_size, out_memory };

        return out_memory;
    }

    ui8* copy_memory(ui8* dst, const ui8* src, ui64 copy_size)
    {
        if(!dst || !src || !copy_size)
        {
            hit_warning_if(!dst, "Attempting to copy memory to an invalid destination!");
            hit_warning_if(!src, "Attempting to copy memory from an invalid source!");
            hit_warning_if(!copy_size, "Attempting to copy 0 memory!");
            return nullptr;
        }

        return (ui8*)std::memcpy(dst, src, copy_size);
    }

    ui8* set_memory(ui8* memory, i32 value, ui64 size)
    {
        if(!memory)
        {
            hit_warning("Attempting to setup an invalid memory!");
            return nullptr;
        }

        std::memset(memory, value, size);

        return memory;
    }

    Usage get_usage(ui8* memory)
    {
        auto memory_usage_location = s_memory_system->usages.find(memory);
        hit_assert(
            memory_usage_location != s_memory_system->usages.end(), 
            "Attempting to get the usage of a not registered memory!");
        
        return memory_usage_location->second;
    }

    bool has_memory(ui8* memory)
    {
        auto memory_usage_location = s_memory_system->usages.find(memory);
        return memory_usage_location != s_memory_system->usages.end();
    }

    Usage allocate_usage(ui64 size, Usage::Type usage)
    {
        auto memory = allocate_memory(size, usage);
        return s_memory_system->usages[memory];
    }

    void deallocate_usage(Usage& usage)
    {
        deallocate_memory(usage.memory);
        usage.memory = nullptr;
        usage.size = 0;
    }

    Usage reallocate_usage(Usage& usage, ui64 new_size)
    {
        auto out_memory = reallocate_memory(usage.memory, new_size);
        if(out_memory)
        {
            usage.memory = out_memory;
            usage.size = new_size;
        }
        return usage;
    }

    Usage copy_usage(Usage& dest, const Usage& src, ui64 copy_size)
    {
        auto out_memory = copy_memory(dest.memory, src.memory, copy_size);
        if(out_memory)
        {
            dest.memory = out_memory;
        }

        return dest;
    }

    Usage set_usage_memory(Usage& usage, i32 value, ui64 size)
    {
        set_memory(usage.memory, value, size);
        return usage;
    }
}