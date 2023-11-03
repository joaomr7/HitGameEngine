#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanRenderer.h"

#include "Core/Engine.h"
#include "Core/Log.h"

#include "Platform/Window.h"

#include <algorithm>

namespace hit
{
	bool VulkanSwapchain::initialize(const VulkanContext context, const VulkanSwapchainInfo& info)
	{
		const auto device = context->get_device();

		const auto& configuration = info.engine->get_renderer_config();
		const auto& swapchain_details = device->get_swapchain_support_details();

		// selcet first format available
		m_format = swapchain_details.formats[0];

		// try to find a best swapchain format
		for(const auto& format : swapchain_details.formats)
		{
			if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				m_format = format;
			}
		}

		// TODO: check present modes support
		// check if vsync is enabled
		if(configuration.vsync)
		{
			m_present_mode = VK_PRESENT_MODE_FIFO_KHR;

			// try to find a best present mode if power saving mode is disabled
			if(!configuration.power_save_mode)
			{
				for(const auto& present_mode : swapchain_details.present_mode)
				{
					if(present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
					{
						m_present_mode = present_mode;
						break;
					}
				}
			}
		}
		else
		{
			m_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}

		// swapchain extent
		if(swapchain_details.capabilities.currentExtent.width != UINT32_MAX)
		{
			m_extent = swapchain_details.capabilities.currentExtent;
		}
		else
		{
			m_extent = { (ui32)info.window->get_width(), (ui32)info.window->get_height() };
		}

		// clamp extent
		m_extent.width = std::clamp(m_extent.width, swapchain_details.capabilities.minImageExtent.width, swapchain_details.capabilities.maxImageExtent.width);
		m_extent.height = std::clamp(m_extent.height, swapchain_details.capabilities.minImageExtent.height, swapchain_details.capabilities.maxImageExtent.height);

		// image count
		m_image_count = swapchain_details.capabilities.minImageCount + 1;
		if(swapchain_details.capabilities.maxImageCount > 0 && m_image_count > swapchain_details.capabilities.maxImageCount)
		{
			m_image_count = swapchain_details.capabilities.maxImageCount;
		}

		m_max_frames_in_flight = m_image_count - 1;

		// swapchain create info
		VkSwapchainCreateInfoKHR swapchain_info{ };
		swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		swapchain_info.surface = device->get_surface();
		swapchain_info.minImageCount = m_image_count;
		swapchain_info.imageFormat = m_format.format;
		swapchain_info.imageColorSpace = m_format.colorSpace;
		swapchain_info.imageExtent = m_extent;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// setup queue indices
		auto queue_indices = device->get_device_queues_indices();
		ui32 queue_family_indices_array[2] = { (ui32)queue_indices.graphics_index, (ui32)queue_indices.present_index };

		if(queue_indices.graphics_index != queue_indices.present_index)
		{
			swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_info.queueFamilyIndexCount = 2;
			swapchain_info.pQueueFamilyIndices = queue_family_indices_array;
		}
		else
		{
			swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_info.queueFamilyIndexCount = 0;
			swapchain_info.pQueueFamilyIndices = nullptr;
		}

		swapchain_info.preTransform = swapchain_details.capabilities.currentTransform;
		swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_info.presentMode = m_present_mode;
		swapchain_info.clipped = VK_TRUE;
		swapchain_info.oldSwapchain = nullptr;

		auto swapchain_creation_result = vkCreateSwapchainKHR(
			device->get_device(),
			&swapchain_info,
			device->get_alloc_callback(),
			&m_swapchain);

		if(!check_vk_result(swapchain_creation_result))
		{
			hit_error("Failed to create vulkan swapchain!");
			return false;
		}

		// get swapchain images
		vkGetSwapchainImagesKHR(device->get_device(), m_swapchain, &m_image_count, nullptr);

		std::vector<VkImage> images(m_image_count);
		vkGetSwapchainImagesKHR(device->get_device(), m_swapchain, &m_image_count, images.data());

		// create swapchain textures, check if textures are already created
		if(m_images.empty())
		{
			m_images.resize(m_image_count);

			for(ui32 i = 0; auto & image_texture : m_images)
			{
				image_texture = create_ref<VulkanTexture>(context);

				if(!image_texture->create_wraper(images[i++], nullptr, m_format.format, m_extent.width, m_extent.height, 4))
				{
					hit_error("Failed to create swapchain textures!");
					return false;
				}
			}
		}
		else
		{
			for(ui32 i = 0; auto& image_texture : m_images)
			{
				if(!image_texture->create_wraper(images[i++], nullptr, m_format.format, m_extent.width, m_extent.height, 4))
				{
					hit_error("Failed to create swapchain textures!");
					return false;
				}
			}
		}

		return true;
	}

	void VulkanSwapchain::shutdown(VulkanDevice* device)
	{
		if(!m_images.empty())
		{
			for(auto& image : m_images)
			{
				image->destroy();
				image = nullptr;
			}

			m_images.clear();
			m_image_count = 0;
			m_max_frames_in_flight = 0;
		}

		if(m_swapchain)
		{
			vkDestroySwapchainKHR(device->get_device(), m_swapchain, device->get_alloc_callback());
			m_swapchain = nullptr;
		}
	}

	bool VulkanSwapchain::recreate(const VulkanContext context, const VulkanSwapchainInfo& info)
	{
		if(!context)
		{
			hit_error("Recreating swapchain with invalid context!");
			return false;
		}

		const VulkanDevice* device = context->get_device();

		if(!m_images.empty())
		{
			for(auto& image : m_images)
			{
				image->destroy();
			}
		}

		if(m_swapchain)
		{
			vkDestroySwapchainKHR(device->get_device(), m_swapchain, device->get_alloc_callback());
			m_swapchain = nullptr;
		}

		return initialize(context, info);
	}
}