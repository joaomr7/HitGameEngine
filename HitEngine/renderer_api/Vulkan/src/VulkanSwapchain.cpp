#include "VulkanSwapchain.h"
#include "VulkanDevice.h"

#include "Core/Engine.h"
#include "Core/Log.h"

#include "Platform/Window.h"

#include <algorithm>

namespace hit
{
	bool VulkanSwapchain::initialize(const VulkanSwapchainInfo& info)
	{
		const auto& configuration = info.engine->get_renderer_config();
		const auto& swapchain_details = info.device->get_swapchain_support_details();

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

		swapchain_info.surface = info.device->get_surface();
		swapchain_info.minImageCount = m_image_count;
		swapchain_info.imageFormat = m_format.format;
		swapchain_info.imageColorSpace = m_format.colorSpace;
		swapchain_info.imageExtent = m_extent;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// setup queue indices
		auto queue_indices = info.device->get_device_queues_indices();
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
			info.device->get_device(),
			&swapchain_info,
			info.device->get_alloc_callback(),
			&m_swapchain);

		if(!check_vk_result(swapchain_creation_result))
		{
			hit_error("Failed to create vulkan swapchain!");
			return false;
		}

		// get swapchain images
		vkGetSwapchainImagesKHR(info.device->get_device(), m_swapchain, &m_image_count, nullptr);
		m_images.resize(m_image_count);
		vkGetSwapchainImagesKHR(info.device->get_device(), m_swapchain, &m_image_count, m_images.data());

		// create image views
		m_views.resize(m_image_count);
		for(ui32 i = 0; i < m_image_count; i++)
		{
			VkImageViewCreateInfo view_info{ };
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = m_images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = m_format.format;

			view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			auto view_creation_result = vkCreateImageView(
				info.device->get_device(),
				&view_info,
				info.device->get_alloc_callback(),
				&m_views[i]);

			if(!check_vk_result(view_creation_result))
			{
				hit_error("Failed to create swapchain image view!");
				return false;
			}
		}

		return true;
	}

	void VulkanSwapchain::shutdown(VulkanDevice* device)
	{ 
		if(!m_views.empty())
		{
			for(auto& view : m_views)
			{
				vkDestroyImageView(device->get_device(), view, device->get_alloc_callback());
			}

			m_views.clear();
			m_images.clear();
			m_image_count = 0;
		}

		if(m_swapchain)
		{
			vkDestroySwapchainKHR(device->get_device(), m_swapchain, device->get_alloc_callback());
			m_swapchain = nullptr;
		}
	}
}