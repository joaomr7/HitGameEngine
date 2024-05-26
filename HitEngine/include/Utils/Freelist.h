#pragma once

#include "Core/Types.h"
#include <map>

namespace hit
{
	class FreelistCore
	{
	public:
		FreelistCore(ui64 initial_size = sizeof(ui64));

		bool push_size(ui64 size, ui64& out_offset);
		bool pop_size(ui64 size, ui64 offset);

		bool resize(ui64 new_size);
		void clear();

		inline ui64 get_free_space() { return m_size - m_used; }

	private:
		std::map<ui64, ui64> m_memory_dist;
		ui64 m_size;
		ui64 m_used;
	};
}