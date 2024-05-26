#pragma once

#include "HandleList.h"

namespace hit
{
    // FastHandleList is a HandleList with resources storage with FastTypedArena
    template<typename T>
    class FastHandleList
    {
    public:
        inline constexpr FastHandleList(ui64 initial_capacity = 32, MemoryUsage usage = MemoryUsage::Any);
        FastHandleList(FastHandleList<T>&& other) noexcept;

        ~FastHandleList();

        FastHandleList<T>& operator=(FastHandleList<T>&& other) noexcept;

        template<typename... Args>
        Handle<T> emplace(Args&&... args);
        Handle<T> add(const T& t);

        void remove(const Handle<T>& handle);
        
        inline T* get(const Handle<T>& handle);

        void reset();

        inline ui64 size();
        inline std::span<T> data();

    private:
        struct Slot
        {
            ui32 resource_index;
			ui32 version;

			i32 next;
			i32 back;
        };

        void link_slots(Slot* a, Slot* b);
        void remove_slot(Slot* slot);
        void replace_slot(Slot* old_slot, Slot* new_slot);

    private:
        FastTypedArena<T> m_resources;
        FastTypedArena<Slot> m_slots;

        i32 m_free_slot_count;
		i32 m_free_slot_start;
		i32 m_last_slot;
		i32 m_penultimate_slot;
    };

    template<typename T>
    inline constexpr FastHandleList<T>::FastHandleList(ui64 initial_capacity, MemoryUsage usage)
        : m_resources(initial_capacity, usage),
        m_slots(initial_capacity, MemoryUsage::Handle_List),
        m_free_slot_count(0),
		m_free_slot_start(-1),
		m_last_slot(-1),
		m_penultimate_slot(-1) { }

    template<typename T>
    FastHandleList<T>::FastHandleList(FastHandleList<T>&& other) noexcept
        : m_resources(std::move(other.m_resources)), 
        m_slots(std::move(other.m_slots)),
        m_free_slot_count(other.m_free_slot_count),
        m_free_slot_start(other.m_free_slot_start),
        m_last_slot(other.m_last_slot),
        m_penultimate_slot(other.m_penultimate_slot)
    {
        other.m_free_slot_count = 0;
		other.m_free_slot_start = -1;
		other.m_last_slot = -1;
		other.m_penultimate_slot = -1;
    }

    template<typename T>
    FastHandleList<T>::~FastHandleList()
    {
        reset();
    }

    template<typename T>
    FastHandleList<T>& FastHandleList<T>::operator=(FastHandleList<T>&& other) noexcept
    {
        reset();

        m_resources == std::move(other.m_resources);
		m_slots == std::move(other.m_slots);

		m_free_slot_count = other.m_free_slot_count;
		m_free_slot_start = other.m_free_slot_start;
		m_last_slot = other.m_last_slot;
		m_penultimate_slot = other.m_penultimate_slot;

		other.m_free_slot_count = 0;
		other.m_free_slot_start = -1;
		other.m_last_slot = -1;
		other.m_penultimate_slot = -1;

		return *this;
    }

    template<typename T>
    template<typename... Args>
    Handle<T> FastHandleList<T>::emplace(Args&&... args)
    {
        Slot* slot;

		if(m_free_slot_count > 0)
		{
			m_free_slot_count--;
			slot = (Slot*)m_slots[m_free_slot_start];
			m_free_slot_start = slot->next;
		}
		else
		{
			slot = (Slot*)m_slots.push_back();
			slot->version = 0;
		}

		slot->next = -1;
		slot->back = -1;

		auto resource = m_resources.emplace_back(std::forward<Args>(args)...);
		slot->resource_index = m_resources.size() - 1;

		Slot* back = nullptr;

		if(m_last_slot != m_penultimate_slot)
		{
			m_penultimate_slot = m_last_slot;
			slot->back = m_penultimate_slot;
			back = (Slot*)m_slots[m_penultimate_slot];
		}

		m_last_slot = (i32)((i32)((ui8*)slot - (ui8*)m_slots.data().data()) / sizeof(Slot));

		if(back) back->next = m_last_slot;

		Handle<T> handle;
		handle.index = m_last_slot;
		handle.version = slot->version;

		return handle;
    }

    template<typename T>
    Handle<T> FastHandleList<T>::add(const T& t)
    {
        return emplace(t);
    }

    template<typename T>
    void FastHandleList<T>::remove(const Handle<T>& handle)
    {
        if(m_last_slot == -1) return;

		auto resource = get(handle);

		if(!resource) return;

		Slot* last_slot = (Slot*)m_slots[m_last_slot];
		Slot* removed_slot = (Slot*)m_slots[handle.index];

		removed_slot->version++;

		if(last_slot == removed_slot)
		{
			m_resources.pop_back();
		}
		else
		{
			T* last_resource = (T*)m_resources[last_slot->resource_index];

            Memory::copy_memory((ui8*)resource, (ui8*)last_resource, sizeof(T));
            Memory::set_memory((ui8*)last_resource, 0, sizeof(T));

			m_resources.pop_back();

			last_slot->resource_index = removed_slot->resource_index;
		}

		if(handle.index == m_penultimate_slot)
		{
			m_penultimate_slot = removed_slot->back;
			last_slot->back = m_penultimate_slot;
		}
		else
		{
			m_last_slot = last_slot->back;

			replace_slot(removed_slot, last_slot);

			if(m_last_slot >= 0)
			{
				last_slot = (Slot*)m_slots[m_last_slot];

				m_penultimate_slot = last_slot->back;
			}
		}

		removed_slot->next = m_free_slot_start;
		removed_slot->back = -1;

		m_free_slot_start = handle.index;
		m_free_slot_count++;
    }
        
    template<typename T>
    inline T* FastHandleList<T>::get(const Handle<T>& handle)
    {
        if(handle.index >= m_slots.size()) return nullptr;

		const Slot* slot = m_slots[handle.index];

		T* resource = nullptr;

		if(handle.version == slot->version)
			resource = (T*)m_resources[slot->resource_index];

		return resource;
    }

    template<typename T>
    void FastHandleList<T>::reset()
    {
        m_resources.clear();
        m_slots.clear();

        m_free_slot_count = 0;
		m_free_slot_start = -1;
		m_last_slot = -1;
		m_penultimate_slot = -1;
    }

    template<typename T>
    inline ui64 FastHandleList<T>::size()
    {
        return m_resources.size();
    }

    template<typename T>
    inline std::span<T> FastHandleList<T>::data()
    {
        return m_resources.data();
    }

    template<typename T>
    void FastHandleList<T>::link_slots(Slot* a, Slot* b)
    {
        a->next = (i32)((i32)((ui8*)b - (ui8*)m_slots.data().data()) / sizeof(Slot));
		b->back = (i32)((i32)((ui8*)a - (ui8*)m_slots.data().data()) / sizeof(Slot));
    }

    template<typename T>
    void FastHandleList<T>::remove_slot(Slot* slot)
    {
        Slot* next = nullptr;
		Slot* back = nullptr;

		if(slot->next != -1)
			next = (Slot*)m_slots[slot->next];

		if(slot->back != -1)
			back = (Slot*)m_slots[slot->back];

		if(next && back)
		{
			link_slots(back, next);
		}
		else
		{
			if(next) next->back = -1;

			if(back) back->next = -1;
		}

		slot->next = -1;
		slot->back = -1;
    }

    template<typename T>
    void FastHandleList<T>::replace_slot(Slot* old_slot, Slot* new_slot)
    {
        remove_slot(new_slot);

		if(old_slot == new_slot) return;

		Slot* next = nullptr;
		Slot* back = nullptr;

		new_slot->next = old_slot->next;
		new_slot->back = old_slot->back;

		if(old_slot->next != -1)
			next = (Slot*)m_slots[old_slot->next];

		if(old_slot->back != -1)
			back = (Slot*)m_slots[old_slot->back];

		if(next)
			link_slots(new_slot, next);

		if(back)
			link_slots(back, new_slot);

		old_slot->back = -1;
		old_slot->next = -1;
    }
}