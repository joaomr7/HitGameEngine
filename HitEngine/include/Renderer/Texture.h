#pragma once

#include "Core/Types.h"
#include "Releseable.h"

namespace hit
{
	struct SamplerInfo
	{
		enum Filter
		{
			FilterNearest,
			FilterLinear
		};

		enum Wrapper
		{
			WrapperRepeat,
			WrapperMirroredRepeat,
			WraperClampToEdge,
			WraperClampToBorder
		};

		Filter min_filter;
		Filter mag_filter;
		Wrapper wrapper;
	};

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

		enum Type
		{
			Type2D
		};

		Source source;
		Format format;
		Type type;

		bool writable;
		ui32 width;
		ui32 height;
		ui32 channels;

		bool use_sampler;
		SamplerInfo sampler_info;
	};

	class Texture : public Releseable
	{
	public:
		virtual ~Texture() = default;

		virtual bool create(const TextureInfo& info, ui8* data) = 0;

		virtual bool write_data(ui32 x, ui32 y, ui64 size, ui8* data) = 0;

		inline ui32 get_width() const { return m_info.width; }
		inline ui32 get_height() const { return m_info.height; }
		inline ui32 get_channels() const { return m_info.channels; }
		inline ui64 get_size() const { return m_info.width * m_info.height * m_info.channels; }

		inline bool is_writable() const { return m_info.writable; }
		inline bool has_sampler() const { return m_info.use_sampler; }

		inline SamplerInfo::Filter get_sampler_min_filter() const { return m_info.sampler_info.min_filter; }
		inline SamplerInfo::Filter get_sampler_mag_filter() const { return m_info.sampler_info.mag_filter; }
		inline SamplerInfo::Wrapper get_sampler_wrapper() const { return m_info.sampler_info.wrapper; }

		inline TextureInfo::Source get_source() const { return m_info.source; }
		inline TextureInfo::Format get_format() const { return m_info.format; }
		inline TextureInfo::Type get_type() const { return m_info.type; }

	protected:
		TextureInfo m_info;
	};
}