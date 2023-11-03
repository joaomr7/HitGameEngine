#pragma once

#include "Core/Types.h"

namespace hit
{
	struct TextureInfo
	{
		enum Source
		{
			SourceOwn,
			SourceExternal
		};

		enum Format
		{
			FormatRGBA,
			FormatBGRA,
		};

		Source source;
		Format format;
	};

	// TODO: add other functionalities
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void destroy() = 0;

		inline ui32 get_width() const { return m_width; }
		inline ui32 get_height() const { return m_height; }
		inline ui32 get_channels() const { return m_channels; }
		inline ui64 get_size() const { return m_width * m_height * m_channels; }

		inline TextureInfo::Source get_source() const { return m_info.source; }
		inline TextureInfo::Format get_format() const { return m_info.format; }

	protected:
		ui32 m_width;
		ui32 m_height;
		ui32 m_channels;
		TextureInfo m_info;
	};
}