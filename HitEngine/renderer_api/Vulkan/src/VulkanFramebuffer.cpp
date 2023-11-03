#include "VulkanFramebuffer.h"
#include "VulkanRenderer.h"
#include "VulkanRenderpass.h"

#include "Core/Assert.h"

namespace hit
{
	bool VulkanFramebuffer::create(const FramebufferConfig& config)
	{
		hit_assert(m_context, "Creating vulkan framebuffer with invalid context!");
		const auto device = m_context->get_device();

		m_config = config;
		const VulkanRenderpass* vk_pass = (const VulkanRenderpass*)m_config.pass;
		const ui32 attachmnets_count = (ui32)m_config.attachments.size();

		std::vector<VkImageView> attachments(attachmnets_count);
		for(ui32 i = 0; i < attachmnets_count; i++)
		{
			const Ref<VulkanTexture> vk_texture = cast_ref<VulkanTexture>(m_config.attachments[i].attachment);
			attachments[i] = vk_texture->get_view();
		}

		VkFramebufferCreateInfo framebuffer_info{ };
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = vk_pass->get_pass();
		framebuffer_info.attachmentCount = m_config.attachments.size();
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = config.attachment_width;
		framebuffer_info.height = config.attachemnt_height;
		framebuffer_info.layers = 1;

		if(!check_vk_result(vkCreateFramebuffer(device->get_device(), &framebuffer_info, device->get_alloc_callback(), &m_framebufer)))
		{
			hit_error("Failed to create vulkan framebuffer!");
			return false;
		}

		return true;
	}

	void VulkanFramebuffer::destroy()
	{ 
		hit_assert(m_context, "Destroying vulkan framebuffer with invalid context!");
		const auto device = m_context->get_device();

		device->wait_idle();

		if(m_framebufer)
		{
			vkDestroyFramebuffer(device->get_device(), m_framebufer, device->get_alloc_callback());
			m_framebufer = nullptr;
		}
	}
}