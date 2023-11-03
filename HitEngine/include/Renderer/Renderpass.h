#pragma once

#include "Core/Types.h"
#include "Math/Vec4.h"
#include "Utils/Ref.h"

#include "Texture.h"

#include <vector>

namespace hit
{
	class Framebuffer;

	struct Attachment
	{
		enum Type
		{
			TypeColor,
			TypeDepth
		};

		enum LoadOperation
		{
			OpLoad,
			OpClear,
			OpLoadDontCare
		};

		enum StoreOperation
		{
			OpStore,
			OpStoreDontCare
		};

		Type type;

		LoadOperation load_op;
		StoreOperation store_op;

		TextureInfo::Format format;

		// can be one or more, depends on how many swapchin images
		std::vector<Ref<Texture>> attachments;
	};

	struct RenderpassConfig
	{
		enum ClearFlag : ui8
		{
			ClearNone    = 0x0,
			ClearColor   = 0x1,
			ClearDepth   = 0x2,
			ClearStencil = 0x4
		};

		f32 depth;
		ui32 stencil;

		Vec4 render_area;
		Vec4 clear_color;

		ui8 clear_flag;

		ui32 attachment_width;
		ui32 attachemnt_height;

		bool present_after;

		std::vector<Attachment> attachments;
	};

	class Renderpass
	{
	public:
		virtual ~Renderpass() = default;

		virtual bool create(const RenderpassConfig& config) = 0;
		virtual void destroy() = 0;

		virtual bool resize(ui32 new_width, ui32 new_height) = 0;

		virtual bool begin() = 0;
		virtual void end() = 0;

		virtual const Ref<Framebuffer> get_current_frame_framebuffer() const = 0;
		inline const Ref<Framebuffer> get_framebuffer(ui32 index) const { return m_framebuffers[index]; }

		inline const RenderpassConfig& get_config() const { return m_config; }

	protected:
		RenderpassConfig m_config;
		std::vector<Ref<Framebuffer>> m_framebuffers;
	};
}