#include "Utils/Arena.h"

#include "Core/Assert.h"

namespace hit
{
    Arena::Arena(ui64 size, MemoryUsage usage)
    {
        bool creation_result = create(size, usage);
        hit_assert(creation_result, "Failed to create Arena!");
    }

    Arena::Arena(const Arena& other)
    {
        other.copy_to(this);
    }

    Arena::Arena(Arena&& other) noexcept : m_usage(other.m_usage), m_used(other.m_used)
    {
        other.m_usage.memory = nullptr;
        other.m_usage.size = 0;
        other.m_used = 0;
    }

    Arena::~Arena()
    {
        destroy();
    }

    Arena& Arena::operator=(const Arena& other)
    {
        destroy();
        other.copy_to(this);
        return *this;
    }

    Arena& Arena::operator=(Arena&& other) noexcept
    {
        destroy();

        m_usage = other.m_usage;
        m_used = other.m_used;

        other.m_usage.memory = nullptr;
        other.m_usage.size = 0;
        other.m_used = 0;

        return *this;
    }

    bool Arena::create(ui64 size, MemoryUsage usage)
    {
        if(!size)
        {
            hit_warning("Attempting to create an Arena with no size!");
            return false;
        }

        if(m_usage.memory)
        {
            hit_warning("Arena usage '{}' is already created!", m_usage);
            return true;
        }

        m_usage = Memory::allocate_usage(size, usage);
        if(!m_usage.memory)
        {
            hit_error("Failed to allocate memory to Arena!");
            return false;
        }

        m_used = 0;

        return true;
    }

	void Arena::destroy()
    {
        if(m_usage.memory)
        {
            Memory::deallocate_usage(m_usage);
            m_used = 0;
        }
    }
    
    ui8* Arena::push_memory(ui64 size)
    {
        if(m_used + size >= m_usage.size)
		{
			ui64 new_capacity = m_usage.size + size;
			new_capacity = (new_capacity >> 1)  | new_capacity;
			new_capacity = (new_capacity >> 2)  | new_capacity;
			new_capacity = (new_capacity >> 4)  | new_capacity;
			new_capacity = (new_capacity >> 8)  | new_capacity;
			new_capacity = (new_capacity >> 16) | new_capacity;
			new_capacity = (new_capacity >> 32) | new_capacity;
			new_capacity++;

            m_usage = Memory::reallocate_usage(m_usage, new_capacity);
		}

        ui8* mem_ptr = m_usage.memory + m_used;
		m_used += size;
		return mem_ptr;
    }

    void Arena::pop_memory(ui64 size)
    {
        hit_assert(size <= m_used, "Attempting to pop {}bytes, but Arena have used just {}bytes!", size, m_used);
        m_used -= size;
    }

    void Arena::increment_memory(ui64 size)
    {
        hit_warning_if(!size, "Attempting to increment Arena memory by 0!");

        if(m_usage.memory && size > 0)
        {
            m_usage = Memory::reallocate_usage(m_usage, m_usage.size + size);
        }
    }

    void Arena::reset()
    {
        m_used = 0;
    }

    void Arena::copy_to(Arena* other) const
    {
        if(!other)
        {
            hit_warning("Attempting to copy an Arena to an invalid one!");
            return;
        }

        if(other == this || other->m_usage.memory == m_usage.memory)
        {
            hit_warning_if(other == this || other->m_usage.memory == m_usage.memory, 
                            "Attempting to copy a Arena memory to it self!");

            return;
        }

        // destroy other Arena if it's already created
        if(other->m_usage.memory) other->destroy();

        if(!m_usage.memory) return;

        bool creation_result = other->create(m_usage.size, m_usage.usage);
        hit_assert(creation_result, "Failed to allocate memory to copy Arena!");

        Memory::copy_usage(other->m_usage, m_usage, m_usage.size);
        other->m_used = m_used;
    }

}