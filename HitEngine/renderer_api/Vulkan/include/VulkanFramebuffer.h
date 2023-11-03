#pragma once

#include "VulkanCommon.h"
#include "Renderer/Framebuffer.h"

namespace hit
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		inline VulkanFramebuffer(const VulkanContext context) : m_context(context) { }
		~VulkanFramebuffer() = default;

		bool create(const FramebufferConfig& config) override;
		void destroy() override;

		inline const VkFramebuffer get_framebuffer() const { return m_framebufer; }

	private:
		VulkanContext m_context = nullptr;
		VkFramebuffer m_framebufer = nullptr;
	};
}