#pragma once

#include "Core/Types.h"
#include "VulkanCommon.h"

#include <vector>

namespace hit
{
	struct VulkanSwapchainInfo
	{
		class Engine* engine;
		class Window* window;
		class VulkanDevice* device;
	};

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain() = default;
		~VulkanSwapchain() = default;

		bool initialize(const VulkanSwapchainInfo& info);
		void shutdown(class VulkanDevice* device);

		inline ui32 get_image_count() const { return m_image_count; }
		inline ui32 get_max_frames_in_flight() const { return m_max_frames_in_flight; }

		inline const VkSwapchainKHR get_swapchain() const { return m_swapchain; }
		inline const VkSurfaceFormatKHR& get_format() const { return m_format; }
		inline const VkPresentModeKHR& get_present_mode() const { return m_present_mode; }

	private:
		VkSwapchainKHR m_swapchain = nullptr;

		VkSurfaceFormatKHR m_format;
		VkPresentModeKHR m_present_mode;

		VkExtent2D m_extent;

		ui32 m_image_count;
		ui32 m_max_frames_in_flight;

		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_views;
	};
}