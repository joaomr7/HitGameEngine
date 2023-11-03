#pragma once

#include "VulkanCommon.h"
#include "Renderer/Texture.h"

#include "Utils/Ref.h"

namespace hit
{
	class VulkanTexture : public Texture
	{
	public:
		inline VulkanTexture(const VulkanContext context) : m_context(context) { }
		~VulkanTexture() = default;

		bool create_wraper(VkImage external_image, VkDeviceMemory exteral_memory, VkFormat format, ui32 width, ui32 height, ui8 channels);
		void destroy() override;

		VkFormat get_vulkan_format() const;

		inline const VkImage get_image() const { return m_image; }
		inline const VkImageView get_view() const { return m_view; }

	private:
		bool create_view();

	private:
		VulkanContext m_context = nullptr;

		VkImage m_image = nullptr;
		VkDeviceMemory m_memory = nullptr;
		VkImageView m_view = nullptr;
	};

	TextureInfo::Format vulkan_format_to_texture_format(VkFormat format);
	VkFormat texture_format_to_vulkan_format(TextureInfo::Format format);
}