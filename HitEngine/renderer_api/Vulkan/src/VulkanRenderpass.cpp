#include "VulkanRenderpass.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "VulkanFramebuffer.h"

#include "Core/Assert.h"
#include "Core/Memory.h"

namespace hit::vulkan_helper
{
	static VkAttachmentLoadOp attachment_load_op_to_vulkan_op(Attachment::LoadOperation load_op)
	{
		switch(load_op)
		{
			case Attachment::OpLoad: return VK_ATTACHMENT_LOAD_OP_LOAD;
			case Attachment::OpClear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case Attachment::OpLoadDontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		hit_assert(false, "Invalid Attachment Load Operation!");
	}

	static VkAttachmentStoreOp attachment_store_op_to_vulkan_op(Attachment::StoreOperation store_op)
	{
		switch(store_op)
		{
			case Attachment::OpStore: return VK_ATTACHMENT_STORE_OP_STORE;
			case Attachment::OpStoreDontCare: VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

		hit_assert(false, "Invalid Attachment Store Operation!");
	}
}

namespace hit
{
	bool VulkanRenderpass::create(const RenderpassConfig& config)
	{
		hit_assert(m_context, "Creating vulkan renderpass with invalid context!");
		const auto device = m_context->get_device();

		m_config = config;

		auto& attachments = m_config.attachments;
		const ui32 attachment_count = (ui32)attachments.size();

		std::vector<VkAttachmentDescription> attachments_desc(attachment_count);
		std::vector<VkAttachmentReference> color_attachment_references;
		std::vector<VkAttachmentReference> depth_attachment_references;

		for(ui32 i = 0; i < attachment_count; i++)
		{
			const Attachment& attachment = attachments[i];

			VkAttachmentDescription attachment_desc{ };

			attachment_desc.format = texture_format_to_vulkan_format(attachment.format);
			attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;

			attachment_desc.loadOp = vulkan_helper::attachment_load_op_to_vulkan_op(attachment.load_op);
			attachment_desc.storeOp = vulkan_helper::attachment_store_op_to_vulkan_op(attachment.store_op);

			attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if(attachment.type == Attachment::TypeColor)
			{
				attachment_desc.initialLayout =
					attachment.load_op == Attachment::OpLoad ?
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

				attachment_desc.finalLayout =
					m_config.present_after ?
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			else if(attachment.type == Attachment::TypeDepth)
			{
				attachment_desc.initialLayout =
					attachment.load_op != Attachment::OpLoadDontCare ?
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

				attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				hit_error("Invalid renderpass attachment type!");
				return false;
			}

			attachments_desc[i] = attachment_desc;

			if(attachment.type == Attachment::TypeColor)
			{
				VkAttachmentReference color_reference{ };
				color_reference.attachment = i;
				color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				color_attachment_references.push_back(color_reference);
			}
			else if(attachment.type == Attachment::TypeDepth)
			{
				VkAttachmentReference depth_reference{ };
				depth_reference.attachment = i;
				depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				depth_attachment_references.push_back(depth_reference);
			}
		}

		// create subpass
		const ui32 color_attachments_ref_count = color_attachment_references.size();
		const ui32 depth_attachments_ref_count = depth_attachment_references.size();

		VkSubpassDescription subpass{ };
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpass.colorAttachmentCount = color_attachments_ref_count;
		subpass.pColorAttachments = color_attachments_ref_count > 0 ? color_attachment_references.data() : nullptr;

		subpass.pDepthStencilAttachment = depth_attachments_ref_count > 0 ? depth_attachment_references.data() : nullptr;

		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;

		subpass.pResolveAttachments = nullptr;

		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		// create dependencies
		VkSubpassDependency dependency{ };
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		// create pass
		VkRenderPassCreateInfo pass_info{ };
		pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		pass_info.attachmentCount = attachment_count;
		pass_info.pAttachments = attachments_desc.data();
		pass_info.subpassCount = 1;
		pass_info.pSubpasses = &subpass;
		pass_info.dependencyCount = 1;
		pass_info.pDependencies = &dependency;

		if(!check_vk_result(vkCreateRenderPass(device->get_device(), &pass_info, device->get_alloc_callback(), &m_pass)))
		{
			hit_error("Failed to create vulkan render pass!");
			return false;
		}

		// generate framebuffers
		if(!generate_framebuffers())
		{
			hit_error("Failed to generate renderpass framebuffers!");
			return false;
		}

		return true;
	}

	void VulkanRenderpass::destroy()
	{
		hit_assert(m_context, "Destroying with invalid context!");
		const auto device = m_context->get_device();

		device->wait_idle();

		if(!m_framebuffers.empty())
		{
			for(auto& framebuffer : m_framebuffers)
			{
				framebuffer->destroy();
				framebuffer = nullptr;
			}

			m_framebuffers.clear();
		}

		if(m_pass)
		{
			vkDestroyRenderPass(device->get_device(), m_pass, device->get_alloc_callback());
			m_pass = nullptr;
		}
	}

	bool VulkanRenderpass::begin()
	{
		hit_assert(m_context, "Beginning vulkan pass with invalid context!");

		const auto& command_buffer = m_context->get_graphics_command();
		const auto framebuffer = cast_ref<VulkanFramebuffer>(get_current_frame_framebuffer())->get_framebuffer();

		// update viewport and scissor
		m_context->set_viewport(
			static_cast<i32>(m_config.render_area.x),
			static_cast<i32>(m_config.render_area.y),
			static_cast<i32>(m_config.render_area.width),
			static_cast<i32>(m_config.render_area.height)
		);

		m_context->set_scissor(
			static_cast<i32>(m_config.render_area.x),
			static_cast<i32>(m_config.render_area.y),
			static_cast<i32>(m_config.render_area.width),
			static_cast<i32>(m_config.render_area.height)
		);

		// begin info
		VkRenderPassBeginInfo begin_info{ };
		begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin_info.renderPass = m_pass;
		begin_info.framebuffer = framebuffer;

		// render area
		begin_info.renderArea.offset.x = static_cast<i32>(m_config.render_area.x);
		begin_info.renderArea.offset.y = static_cast<i32>(m_config.render_area.y);
		begin_info.renderArea.extent.width = static_cast<i32>(m_config.render_area.width);
		begin_info.renderArea.extent.height = static_cast<i32>(m_config.render_area.height);

		// clear
		begin_info.clearValueCount = 0;
		begin_info.pClearValues = nullptr;

		VkClearValue clear_values[2]{ };
		const bool clear_color = (m_config.clear_flag & RenderpassConfig::ClearColor) != 0;
		if(clear_color)
		{
			Memory::copy_memory((ui8*)clear_values[begin_info.clearValueCount++].color.float32, (ui8*)m_config.clear_color.elements, sizeof(f32) * 4);
		}
		else
		{
			begin_info.clearValueCount++;
		}

		const bool clear_depth = (m_config.clear_flag & RenderpassConfig::ClearDepth) != 0;
		if(clear_depth)
		{
			clear_values[begin_info.clearValueCount].depthStencil.depth = m_config.depth;

			const bool clear_stencil = (m_config.clear_flag & RenderpassConfig::ClearStencil) != 0;
			clear_values[begin_info.clearValueCount++].depthStencil.stencil = clear_stencil ? m_config.stencil : 0;
		}

		begin_info.pClearValues = clear_values;

		vkCmdBeginRenderPass(command_buffer.get_command_buffer(), &begin_info, VK_SUBPASS_CONTENTS_INLINE);

		return true;
	}

	void VulkanRenderpass::end()
	{ 
		hit_assert(m_context, "Ending vulkan pass with invalid context!");

		const auto& command_buffer = m_context->get_graphics_command();

		vkCmdEndRenderPass(command_buffer.get_command_buffer());
	}

	bool VulkanRenderpass::generate_framebuffers()
	{
		hit_assert(m_context, "Generating framebuffers with invalid context!");
		const auto max_images = m_context->get_image_count();

		for(ui32 i = 0; i < max_images; i++)
		{
			FramebufferConfig framebuffer_config;
			framebuffer_config.pass = this;
			framebuffer_config.attachment_width = m_config.attachment_width;
			framebuffer_config.attachemnt_height = m_config.attachemnt_height;

			// get attachments
			for(const auto& attachment : m_config.attachments)
			{
				FramebufferAttachment framebuffer_attachment;
				framebuffer_attachment.type = attachment.type;

				if(attachment.attachments.size() == max_images)
				{
					framebuffer_attachment.attachment = attachment.attachments[i];
				}
				else
				{
					framebuffer_attachment.attachment = attachment.attachments[0];
				}

				framebuffer_config.attachments.push_back(framebuffer_attachment);
			}

			auto framebuffer = create_ref<VulkanFramebuffer>(m_context);
			if(!framebuffer->create(framebuffer_config))
			{
				hit_error("Failed to create pass framebuffers!");
				return false;
			}

			m_framebuffers.push_back(framebuffer);
		}

		return true;
	}

	bool VulkanRenderpass::resize(ui32 new_width, ui32 new_height)
	{
		RenderpassConfig new_pass_config = get_config();
		new_pass_config.attachment_width = new_width;
		new_pass_config.attachemnt_height = new_height;
		new_pass_config.render_area.width = new_width;
		new_pass_config.render_area.height = new_height;

		destroy();

		return create(new_pass_config);
	}

	const Ref<Framebuffer> VulkanRenderpass::get_current_frame_framebuffer() const
	{
		hit_assert(m_context, "Generating current framebuffer with invalid context!");
		return m_framebuffers[m_context->get_current_image_index()];
	}
}