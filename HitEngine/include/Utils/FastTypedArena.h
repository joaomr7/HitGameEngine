#pragma once

#include "Core/Assert.h"
#include "Core/Types.h"
#include "Arena.h"
#include <span>

namespace hit
{
    // FastTypedArena is a TypedArena which does not initialize its elements
    template <typename T>
    class FastTypedArena
    {
    public:
        FastTypedArena() = default;
        FastTypedArena(ui64 size, MemoryUsage usage = MemoryUsage::Any);
        FastTypedArena(const FastTypedArena& other);
        FastTypedArena(FastTypedArena&& other) noexcept;

        ~FastTypedArena();

        FastTypedArena& operator=(const FastTypedArena& other);
        FastTypedArena& operator=(FastTypedArena&& other) noexcept;

        bool create(ui64 size, MemoryUsage usage = MemoryUsage::Any);
        void destroy();

        void reserve_space(ui64 size);

        template<typename... Args>
        T* emplace_back(Args&&... args);

        T* push_back();
        T* push_back(const T& t);
        T* push_back(T&& t);

        T* push_array(ui64 count);

        void pop_back();
        void pop_array_back(ui64 count);

        void clear();
        inline bool empty() const;

        inline T* get(ui64 index);
        inline const T* get(ui64 index) const;

        inline T* operator[](ui64 index);
        inline const T* operator[](ui64 index) const;

        inline const std::span<T> data() const;

        inline ui64 size() const;
        inline ui64 capacity() const;

    private:
        Arena m_arena;
        ui64 m_capacity = 0;
        ui64 m_size = 0;
    };

    template <typename T>
    FastTypedArena<T>::FastTypedArena(ui64 size, MemoryUsage usage)
    {
        bool creation_result = create(size, usage);
        hit_assert(creation_result, "Failed to create FastTypedArena!");
    }

    template <typename T>
    FastTypedArena<T>::FastTypedArena(const FastTypedArena<T>& other) 
        : m_arena(other.m_arena), m_capacity(other.m_capacity), m_size(other.m_size) { }

    template <typename T>
    FastTypedArena<T>::FastTypedArena(FastTypedArena<T>&& other) noexcept
        : m_arena(std::move(other.m_arena)), m_capacity(other.m_capacity), m_size(other.m_size) 
    {
        other.m_capacity = 0;
        other.m_size = 0;
    }

    template <typename T>
    FastTypedArena<T>::~FastTypedArena()
    {
        destroy();
    }

    template <typename T>
    FastTypedArena<T>& FastTypedArena<T>::operator=(const FastTypedArena<T>& other)
    {
        destroy();

        m_arena = other.m_arena;
        m_capacity = other.m_capacity;
        m_size = other.m_size;

        return *this;
    }

    template <typename T>
    FastTypedArena<T>& FastTypedArena<T>::operator=(FastTypedArena<T>&& other) noexcept
    {
        destroy();

        m_arena = std::move(other.m_arena);
        m_capacity = other.m_capacity;
        m_size = other.m_size;

        other.m_capacity = 0;
        other.m_size = 0;

        return *this;
    }

    template <typename T>
    bool FastTypedArena<T>::create(ui64 size, MemoryUsage usage)
    {
        if(!m_arena.create(size * sizeof(T), usage))
        {
            hit_error("Failed to allocate TypedArena memory!");
            return false;
        }

        m_capacity = size;
        m_size = 0;

        return true;
    }

    template <typename T>
    void FastTypedArena<T>::destroy()
    {
        m_arena.destroy();
        m_capacity = 0;
        m_size = 0;
    }

    template <typename T>
    void FastTypedArena<T>::reserve_space(ui64 size)
    {
        hit_assert(m_capacity > 0, "Can't reserve space to a not initialized yet TypedArena.");

        const ui64 new_total_size = (m_capacity + size) * sizeof(T);

        if(new_total_size > m_arena.capacity())
        {
            const ui64 increment_size = size * sizeof(T);
            m_arena.increment_memory(increment_size);
        }

        m_capacity += size;
    }

    template <typename T>
    template<typename... Args>
    T* FastTypedArena<T>::emplace_back(Args&&... args)
    {
        hit_assert(m_capacity > 0, "Can't reserve space to a not initialized yet TypedArena.");

        if(m_size + 1 > m_capacity)
        {
            // increments capacity by 1 + 25% of current capacity
            reserve_space(1 + (ui64)(m_capacity / 4));
        }

        // allocate and initialize element
        auto out_value = (T*)m_arena.push_memory(sizeof(T));
        *out_value = T(std::forward<Args>(args)...);

        // increment size
        m_size++;

        return out_value;
    }

    template <typename T>
    T* FastTypedArena<T>::push_back()
    {
        hit_assert(m_capacity > 0, "Can't reserve space to a not initialized yet TypedArena.");

        if(m_size + 1 > m_capacity)
        {
            // increments capacity by 1 + 25% of current capacity
            reserve_space(1 + (ui64)(m_capacity / 4));
        }

        // allocate and initialize element
        auto out_value = (T*)m_arena.push_memory(sizeof(T));

        // increment size
        m_size++;

        return out_value;
    }

    template <typename T>
    T* FastTypedArena<T>::push_back(const T& t)
    {
        hit_assert(m_capacity > 0, "Can't reserve space to a not initialized yet TypedArena.");

        if(m_size + 1 > m_capacity)
        {
            // increments capacity by 1 + 25% of current capacity
            reserve_space(1 + (ui64)(m_capacity / 4));
        }

        // allocate and copy element
        auto out_value = (T*)m_arena.push_memory(sizeof(T));
        Memory::copy_memory((ui8*)out_value, (ui8*)&t, sizeof(T));

        // increment size
        m_size++;

        return out_value;
    }

    template <typename T>
    T* FastTypedArena<T>::push_back(T&& t)
    {
        hit_assert(m_capacity > 0, "Can't reserve space to a not initialized yet TypedArena.");

        if(m_size + 1 > m_capacity)
        {
            // increments capacity by 1 + 25% of current capacity
            reserve_space(1 + (ui64)(m_capacity / 4));
        }

        // allocate and initialize element
        auto out_value = (T*)m_arena.push_memory(sizeof(T));

        Memory::copy_memory((ui8*)out_value, (ui8*)&t, sizeof(T));
        Memory::set_memory((ui8*)&t, 0, sizeof(T));

        // increment size
        m_size++;

        return out_value;
    }

    template <typename T>
    T* FastTypedArena<T>::push_array(ui64 count)
    {
        hit_assert(count > 0, "Attempting to push an array in a TypedArena with 0 elements!");
        hit_assert(m_capacity > 0, "Can't reserve space to a not initialized yet TypedArena.");

        if(m_size + count > m_capacity)
        {
            // increments capacity by cout + 25% of current capacity
            reserve_space(count + (ui64)(m_capacity / 4));
        }

        // allocate elements
        auto out_array = (T*)m_arena.push_memory(count * sizeof(T));

        // increment size
        m_size += count;

        return out_array;
    }

    template <typename T>
    void FastTypedArena<T>::pop_back()
    {
        hit_assert(m_capacity, "Attempting to pop an element from a not initialized yet TypedArena.");
        hit_assert(m_size, "Attempting to pop an element from an empty TypedArena.");

        m_size--;
        m_arena.pop_memory(sizeof(T));
    }

    template <typename T>
    void FastTypedArena<T>::pop_array_back(ui64 count)
    {
        hit_assert(m_capacity, "Attempting to pop an element from a not initialized yet TypedArena.");
        hit_assert(m_size, "Attempting to pop an element from an empty TypedArena.");

        m_size -= count;
        m_arena.pop_memory(count * sizeof(T));
    }

    template <typename T>
    void FastTypedArena<T>::clear()
    {
        m_size = 0;
    }

    template <typename T>
    inline bool FastTypedArena<T>::empty() const
    {
        return m_size == 0;
    }

    template <typename T>
    inline T* FastTypedArena<T>::get(ui64 index)
    {
        hit_assert(index < m_size, "Invalid TypedArena index!");
        return &data()[index];
    }

    template <typename T>
    inline const T* FastTypedArena<T>::get(ui64 index) const
    {
        hit_assert(index < m_size, "Invalid TypedArena index!");
        return &data()[index];
    }

    template <typename T>
    inline T* FastTypedArena<T>::operator[](ui64 index)
    {
        hit_assert(index < m_size, "Invalid TypedArena index!");
        return &data()[index];
    }

    template <typename T>
    inline const T* FastTypedArena<T>::operator[](ui64 index) const
    {
        hit_assert(index < m_size, "Invalid TypedArena index!");
        return &data()[index];
    }

    template <typename T>
    inline const std::span<T> FastTypedArena<T>::data() const
    {
        return { (T*)m_arena.memory(), m_size };
    }

    template <typename T>
    inline ui64 FastTypedArena<T>::size() const
    {
        return m_size;
    }

    template <typename T>
    inline ui64 FastTypedArena<T>::capacity() const
    {
        return m_capacity;
    }
}