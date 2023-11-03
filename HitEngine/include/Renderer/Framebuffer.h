#pragma once

#include "Core/Types.h"
#include "Utils/Ref.h"

#include "Texture.h"
#include "Renderpass.h"

#include <vector>

namespace hit
{
	struct FramebufferAttachment
	{
		Attachment::Type type;
		Ref<Texture> attachment;
	};

	struct FramebufferConfig
	{
		Renderpass* pass;
		ui32 attachment_width;
		ui32 attachemnt_height;
		std::vector<FramebufferAttachment> attachments;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual bool create(const FramebufferConfig& config) = 0;
		virtual void destroy() = 0;

		inline const FramebufferConfig& get_config() const { return m_config; }

	protected:
		FramebufferConfig m_config;
	};
}