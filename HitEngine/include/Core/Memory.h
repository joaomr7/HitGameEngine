#pragma once

#include "Types.h"

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

            };

            Type usage;
            ui64 size;
            ui8* memory;
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
        void deallocate_usage(const Usage& usage);

        Usage reallocate_usage(Usage& usage, ui64 new_size);
        Usage copy_usage(Usage& dest, const Usage& src, ui64 copy_size);

        Usage set_usage_memory(Usage& usage, i32 value, ui64 size);
    }
}