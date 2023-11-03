#pragma once

#include "VulkanCommon.h"
#include "Renderer/Renderpass.h"

namespace hit
{
	class VulkanRenderpass : public Renderpass
	{
	public:
		VulkanRenderpass(const VulkanContext context) : m_context(context) { }
		~VulkanRenderpass() = default;

		bool create(const RenderpassConfig& config) override;
		void destroy() override;

		bool begin() override;
		void end() override;

		bool resize(ui32 new_width, ui32 new_height) override;

		const Ref<Framebuffer> get_current_frame_framebuffer() const override;

		inline const VkRenderPass get_pass() const { return m_pass; }

	private:
		bool generate_framebuffers();

	private:
		VulkanContext m_context = nullptr;
		VkRenderPass m_pass = nullptr;
	};
}