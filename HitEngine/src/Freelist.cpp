#include "Utils/Freelist.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <algorithm>

namespace hit
{
	FreelistCore::FreelistCore(ui64 initial_size) : m_size(initial_size), m_used(0)
	{
		m_memory_dist[0] = initial_size;
	}

	bool FreelistCore::push_size(ui64 size, ui64& out_offset)
	{
		auto it = std::lower_bound(m_memory_dist.begin(), m_memory_dist.end(), size,
			[](const std::pair<const ui64, ui64>& elem, ui64 value)
		{
			return elem.second < value;
		});

		if(it == m_memory_dist.end())
		{
			hit_warning("Can't find space to allocate {} bytes", size);
			return false;
		}

		out_offset = it->first;
		ui64 remaining_size = it->second - size;

		m_memory_dist.erase(it);

		if(remaining_size > 0)
		{
			m_memory_dist[out_offset + size] = remaining_size;
		}

		m_used += size;

		return true;
	}

	bool FreelistCore::pop_size(ui64 size, ui64 offset)
	{
		hit_assert(m_used > 0, "Attempting to pop memory without allocating any memory!");
		hit_assert(offset + size <= m_size, "Attempting to pop memory at an out of bound location!");

		// check if offset + size is not going beyond it must go
		auto next_it = m_memory_dist.lower_bound(offset);
		auto prev_it = next_it != m_memory_dist.end() ? std::prev(next_it) : m_memory_dist.end();
		if(next_it != m_memory_dist.end())
		{
			hit_warning_if(offset + size > next_it->first, "Attempting to pop memory at a free space!");
			hit_warning_if(offset + size > next_it->first + next_it->second, "Attempting to pop memory at a free space!");

			ui64 poping_size = size;
			if(offset + size >= next_it->first && offset + size < next_it->first + next_it->second)
			{
				ui64 next_offset, next_size;
				do
				{
					next_offset = next_it->first;
					next_size = next_it->second;

					m_memory_dist.erase(next_it);

					poping_size += next_size;

					next_it = m_memory_dist.lower_bound(offset);

				} while(next_it != m_memory_dist.end() 
						&& next_size + next_offset >= next_it->first
						&& next_size + next_offset < next_it->first + next_it->second);
			}

			if(prev_it != m_memory_dist.end())
			{
				if (prev_it->first + prev_it->second == offset)
				{
					prev_it->second += poping_size;
					m_used -= size;

					return true;
				}
			}

			m_memory_dist[offset] = poping_size;
			m_used -= size;

			return true;
		}

		if (prev_it != m_memory_dist.end())
		{
			if (prev_it->first + prev_it->second == offset)
			{
				prev_it->second += size;
				m_used -= size;

				return true;
			}
		}

		m_memory_dist[offset] = size;
		m_used -= size;

		return true;
	}

	bool FreelistCore::resize(ui64 new_size)
	{
		if(new_size < m_size)
		{
			hit_error("Can't resize a freelist to a size less than it has at the moment.");
			return false;
		}

		if(new_size == m_size) return true;

		auto end = std::prev(m_memory_dist.end());
		if(end != m_memory_dist.end())
		{
			if(end->second == m_size)
			{
				end->second = new_size;
				m_size = new_size;
				return true;
			}
			else if(end->first + end->second == m_size)
			{
				end->second += new_size - m_size;
				m_size = new_size;
				return true;
			}

		}
		
		m_memory_dist[m_size] = new_size - m_size;
		m_size = new_size;

		return true;
	}

	void FreelistCore::clear()
	{
		m_memory_dist.clear();
		m_memory_dist[0] = m_size;
		m_used = 0;
	}
}