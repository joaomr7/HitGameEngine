#pragma once

#include "Core/Memory.h"

namespace hit
{
    class Arena
    {
    public:
        Arena() = default;
        Arena(ui64 size, MemoryUsage usage = MemoryUsage::Any);
        Arena(const Arena& other);
        Arena(Arena&& other) noexcept;

        ~Arena();

        Arena& operator=(const Arena& other);
        Arena& operator=(Arena&& other) noexcept;

        bool create(ui64 size, MemoryUsage usage = MemoryUsage::Any);
		void destroy();

		ui8* push_memory(ui64 size);
		void pop_memory(ui64 size);
		void increment_memory(ui64 size);

        void reset();
        void copy_to(Arena* other) const;

        inline ui64 size() const { return m_used; }
        inline ui64 capacity() const { return m_usage.size; }
        inline const ui8* memory() const { return m_usage.memory; }

        inline MemoryUsage usage() const { return m_usage.usage; }

    private:
        Memory::Usage m_usage;
        ui64 m_used = 0;
    };
}