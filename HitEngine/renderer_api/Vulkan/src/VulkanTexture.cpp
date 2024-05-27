#include "VulkanTexture.h"
#include "VulkanRenderer.h"

#include "Core/Assert.h"
#include "VulkanBuffer.h"
#include "VulkanCommand.h"

namespace hit
{
	bool VulkanTexture::create(const TextureInfo& info, ui8* data)
	{
		m_info = info;

		if (get_size() == 0)
		{
			hit_error("Can't create texture with 0 size.");
			return false;
		}

		if (!data)
		{
			hit_error("Can't create texture with null data.");
			return false;
		}

		if (!create_image())
		{
			destroy();

			hit_error("Failed to create texture renderer image.");
			return false;
		}

		if (!allocate_memory())
		{
			destroy();

			hit_error("Failed to allocate renderer image memory.");
			return false;
		}

		if (!create_view())
		{
			destroy();

			hit_error("Failed to create texture renderer view.");
			return false;
		}

		if (!write_data(0, 0, get_size(), data))
		{
			destroy();

			hit_error("Failed to write texture data.");
			return false;
		}

		if (has_sampler())
		{
			if (!create_sampler())
			{
				destroy();

				hit_error("Failed to create texture sampler.");
				return false;
			}
		}

		return true;
	}

	bool VulkanTexture::create_wraper(VkImage external_image, VkDeviceMemory exteral_memory, VkFormat format, ui32 width, ui32 height, ui8 channels)
	{
		m_image = external_image;
		m_memory = exteral_memory;

		m_info.format = vulkan_format_to_texture_format(format);
		m_info.source = TextureInfo::SourceExternal;
		m_info.width = width;
		m_info.height = height;
		m_info.channels = (ui32)channels;

		m_info.type = TextureInfo::Type2D;
		m_info.use_sampler = false;

		if (get_size() == 0)
		{
			hit_error("Can't create texture with 0 size.");
			return false;
		}

		if (!create_view())
		{
			destroy();

			hit_error("Failed to create texture renderer view.");
			return false;
		}

		return true;
	}

	void VulkanTexture::destroy()
	{ 
		hit_assert(m_context, "Destroying with invalid context!");
		const auto device = m_context->get_device();

		device->wait_idle();

		if (m_view)
		{
			vkDestroyImageView(device->get_device(), m_view, device->get_alloc_callback());
		}

		if(get_source() == TextureInfo::SourceOwn)
		{
			if (m_image)
			{
				vkDestroyImage(device->get_device(), m_image, device->get_alloc_callback());
			}

			if (m_memory)
			{
				vkFreeMemory(device->get_device(), m_memory, device->get_alloc_callback());
			}
		}

		if (m_sampler)
		{
			vkDestroySampler(device->get_device(), m_sampler, device->get_alloc_callback());
		}

		m_image = nullptr;
		m_memory = nullptr;
		m_view = nullptr;
		m_sampler = nullptr;
	}

	bool VulkanTexture::write_data(ui32 x, ui32 y, ui64 size, ui8* data)
	{
		hit_assert(m_context, "Writing vulkan image data with invalid context!");
		const auto device = m_context->get_device();

		auto stagging = cast_ref<VulkanBuffer>(m_context->acquire_buffer());
		if (!stagging->create(size, BufferType::Staging, BufferAllocationType::None))
		{
			hit_error("Failed to create image staging buffer.");
			return false;
		}

		if (!stagging->bind())
		{
			hit_error("Failed to bind texture stagging buffer.");
			return false;
		}

		if (!stagging->load(0, size, data))
		{
			stagging->destroy();

			hit_error("Failed to load data to image staging buffer.");
			return false;
		}

		bool result = true;
		run_single_graphics_command((VulkanDevice*)device, [&](auto command)
		{
			if (!transtition_layout(command, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
			{
				result = false;
				return;
			}

			VkBufferImageCopy region {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { (i32)x, (i32)y, 0 };
			region.imageExtent.width = get_width();
			region.imageExtent.height = get_height();
			region.imageExtent.depth = 1;

			vkCmdCopyBufferToImage(command, stagging->get_buffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			if (!transtition_layout(command, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
			{
				result = false;
				return;
			}
		});

		stagging->unbind();

		stagging->destroy();

		return true;
	}

	VkFormat VulkanTexture::get_vulkan_format() const
	{
		return texture_format_to_vulkan_format(get_format());
	}

	bool VulkanTexture::create_image()
	{
		hit_assert(m_context, "Creating vulkan image with invalid context!");
		const auto device = m_context->get_device();

		VkImageCreateInfo image_info { };
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

		switch (m_info.type)
		{
			default:
			case TextureInfo::Type2D:
			{
				image_info.imageType = VK_IMAGE_TYPE_2D;
				break;
			}
		}

		image_info.format = texture_format_to_vulkan_format(get_format());
		image_info.extent.width = get_width();
		image_info.extent.height = get_height();
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (is_writable())
		{
			image_info.usage = 
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		else
		{
			image_info.usage =
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT;
		}

		auto result = vkCreateImage(
			device->get_device(),
			&image_info,
			device->get_alloc_callback(),
			&m_image);

		if (!check_vk_result(result))
		{
			hit_error("Failed to create vulkan image!");
			return false;
		}

		return true;
	}

	bool VulkanTexture::allocate_memory()
	{
		hit_assert(m_context, "Allocating vulkan image memory with invalid context!");
		const auto device = m_context->get_device();

		// Get memory requirements
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(device->get_device(), m_image, &requirements);

		auto memory_type = device->get_memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (memory_type == -1)
		{
			hit_error("Cant'find valid image memory type.");
			return false;
		}

		VkMemoryAllocateInfo allocate_info { };
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = requirements.size;
		allocate_info.memoryTypeIndex = memory_type;
		
		auto result = vkAllocateMemory(
			device->get_device(),
			&allocate_info,
			device->get_alloc_callback(),
			&m_memory);

		if (!check_vk_result(result))
		{
			hit_error("Failed to allocate image memory.");
			return false;
		}

		if (!check_vk_result(vkBindImageMemory(device->get_device(), m_image, m_memory, 0)))
		{
			hit_error("Failed to bind image to memory.");
			return false;
		}

		return true;
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

	bool VulkanTexture::create_sampler()
	{
		hit_assert(m_context, "Creating texture sampler with invalid context!");
		const auto device = m_context->get_device();

		VkSamplerCreateInfo sampler_info {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.minFilter = sampler_filter_to_vulkan_filter(get_sampler_min_filter());
		sampler_info.magFilter = sampler_filter_to_vulkan_filter(get_sampler_mag_filter());

		auto vk_address_mode = sampler_wrapper_to_vulkan_adress_mode(get_sampler_wrapper());

		sampler_info.addressModeU = vk_address_mode;
		sampler_info.addressModeV = vk_address_mode;
		sampler_info.addressModeW = vk_address_mode;

		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy = device->get_device_details().properties.limits.maxSamplerAnisotropy;

		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		sampler_info.unnormalizedCoordinates = VK_FALSE;
		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;

		auto result = vkCreateSampler(device->get_device(), &sampler_info, device->get_alloc_callback(), &m_sampler);

		if (!check_vk_result(result))
		{
			hit_error("Failed to create vulkan texture sampler.");
			return false;
		}

		return true;
	}

	bool VulkanTexture::transtition_layout(VkCommandBuffer command, VkImageLayout old_layout, VkImageLayout new_layout)
	{
		hit_assert(m_context, "Doing image transition layout with invalid context!");
		const auto device = m_context->get_device();

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags dest_stage;

		VkImageMemoryBarrier barrier { };
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = device->get_graphics_queue_index();
		barrier.dstQueueFamilyIndex = device->get_graphics_queue_index();
		barrier.image = m_image;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		}
		else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else
		{
			hit_fatal("Unsupported layout transition value.");
			return false;
		}

		vkCmdPipelineBarrier(
			command,
			source_stage, dest_stage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

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
			case TextureInfo::FormatRGBA: return VK_FORMAT_R8G8B8A8_UNORM;
			case TextureInfo::FormatBGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		}
	}

	VkFilter sampler_filter_to_vulkan_filter(SamplerInfo::Filter filter)
	{
		switch (filter)
		{
			case SamplerInfo::FilterNearest: return VK_FILTER_NEAREST;
			case SamplerInfo::FilterLinear:  return VK_FILTER_LINEAR;
		}
	}

	VkSamplerAddressMode sampler_wrapper_to_vulkan_adress_mode(SamplerInfo::Wrapper wrapper)
	{
		switch (wrapper)
		{
			case SamplerInfo::WrapperRepeat:          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case SamplerInfo::WrapperMirroredRepeat:  return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case SamplerInfo::WraperClampToEdge:      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case SamplerInfo::WraperClampToBorder:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}
	}
}