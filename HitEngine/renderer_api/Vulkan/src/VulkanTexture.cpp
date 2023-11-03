#include "VulkanTexture.h"
#include "VulkanRenderer.h"

#include "Core/Assert.h"

namespace hit
{
	bool VulkanTexture::create_wraper(VkImage external_image, VkDeviceMemory exteral_memory, VkFormat format, ui32 width, ui32 height, ui8 channels)
	{
		m_image = external_image;
		m_memory = exteral_memory;

		m_width = width;
		m_height = height;
		m_channels = (ui32)channels;

		m_info.format = vulkan_format_to_texture_format(format);
		m_info.source = TextureInfo::SourceExternal;

		return create_view();
	}

	void VulkanTexture::destroy()
	{ 
		hit_assert(m_context, "Destroying with invalid context!");
		const auto device = m_context->get_device();

		device->wait_idle();

		if(get_source() == TextureInfo::SourceOwn)
		{

		}
		else
		{
			// destroy just view
			if(m_view)
			{
				vkDestroyImageView(device->get_device(), m_view, device->get_alloc_callback());
			}
		}

		m_image = nullptr;
		m_memory = nullptr;
		m_view = nullptr;
	}

	VkFormat VulkanTexture::get_vulkan_format() const
	{
		return texture_format_to_vulkan_format(get_format());
	}

	bool VulkanTexture::create_view()
	{ 
		hit_assert(m_context, "Creating vulkan image view with invalid context!");
		const auto device = m_context->get_device();

		// TODO: add custom configurations
		VkImageViewCreateInfo view_info{ };
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = m_image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = texture_format_to_vulkan_format(get_format());

		view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		auto result = vkCreateImageView(
			device->get_device(),
			&view_info,
			device->get_alloc_callback(),
			&m_view
		);

		if(!check_vk_result(result))
		{
			hit_error("Failed to create vulkan image view!");
			return false;
		}

		return true;
	}

	TextureInfo::Format vulkan_format_to_texture_format(VkFormat format)
	{
		switch(format)
		{
			case VK_FORMAT_R8G8B8A8_UNORM: return TextureInfo::FormatRGBA;
			case VK_FORMAT_B8G8R8A8_UNORM: return TextureInfo::FormatBGRA;
			default: hit_assert(false, "Invalid vulkan format!");
		}
	}

	VkFormat texture_format_to_vulkan_format(TextureInfo::Format format)
	{
		switch(format)
		{
			case hit::TextureInfo::FormatRGBA: return VK_FORMAT_R8G8B8A8_UNORM;
			case hit::TextureInfo::FormatBGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		}
	}
}