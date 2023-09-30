#pragma once

#include "Types.h"
#include "Log.h"

namespace hit
{
    // TODO: make it thread safe
    namespace Memory
    {
        struct Usage
        {
            enum Type
            {
                Any,
                Handle_List,
            };

            Type usage = Any;
            ui64 size = 0;
            ui8* memory = nullptr;
        };

        bool initialize_memory_system();
        bool shutdown_memory_system();

        ui8* allocate_memory(ui64 size, Usage::Type usage);
        void deallocate_memory(ui8* memory);

        ui8* reallocate_memory(ui8* memory, ui64 new_size);
        ui8* copy_memory(ui8* dst, const ui8* src, ui64 copy_size);

        ui8* set_memory(ui8* memory, i32 value, ui64 size);

        Usage get_usage(ui8* memory);

        Usage allocate_usage(ui64 size, Usage::Type usage);
        void deallocate_usage(Usage& usage);

        Usage reallocate_usage(Usage& usage, ui64 new_size);
        Usage copy_usage(Usage& dest, const Usage& src, ui64 copy_size);

        Usage set_usage_memory(Usage& usage, i32 value, ui64 size);
    }

    using MemoryUsage = Memory::Usage::Type;
}

//Usage custom string format
template<>
struct std::formatter<hit::MemoryUsage>
{
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(hit::MemoryUsage usage, std::format_context& ctx) {
        switch (usage)
        {
            case hit::MemoryUsage::Any: 
                return std::format_to(ctx.out(), "Any");

            case hit::MemoryUsage::Handle_List:
                return std::format_to(ctx.out(), "Handle_List");

            default: 
                return std::format_to(ctx.out(), "None");
        }
    }
};

template<>
struct std::formatter<hit::Memory::Usage> 
{
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const hit::Memory::Usage& usage, std::format_context& ctx) {
        return std::format_to(ctx.out(), "Usage[{}], size:{}, type:{}", (void*)usage.memory, usage.size, usage.usage);
    }
};