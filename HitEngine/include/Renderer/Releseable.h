#pragma once

#include "Core/Types.h"

namespace hit
{
	class Releseable
	{
	public:
		enum ReleaseMode : ui8
		{
			ModeAtFirstOpportunity,
			ModeByOwner
		};

	public:
		virtual ~Releseable() = default;

		inline void safe_release()
		{
			if (!m_is_released && m_release_mode == ModeAtFirstOpportunity)
			{
				destroy();
				m_is_released = true;
			}
		}

		inline void force_release()
		{
			destroy();
			m_is_released = true;
		}

		inline void set_release_mode(ReleaseMode mode) { m_release_mode = mode; }

		inline bool is_released() const { return m_is_released; }

	protected:
		virtual void destroy() = 0;

	private:
		ReleaseMode m_release_mode = ModeByOwner;
		bool m_is_released = false;
	};
}