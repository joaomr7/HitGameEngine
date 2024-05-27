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

		bool create(const TextureInfo& info, ui8* data) override;
		bool create_wraper(VkImage external_image, VkDeviceMemory exteral_memory, VkFormat format, ui32 width, ui32 height, ui8 channels);

		bool write_data(ui32 x, ui32 y, ui64 size, ui8* data) override;

		VkFormat get_vulkan_format() const;

		inline const VkImage get_image() const { return m_image; }
		inline const VkImageView get_view() const { return m_view; }

		inline const VkSampler get_sampler() const { return m_sampler; }

	protected:
		void destroy() override;

	private:
		bool create_image();
		bool allocate_memory();
		bool create_view();

		bool create_sampler();

		bool transtition_layout(VkCommandBuffer command, VkImageLayout old_layout, VkImageLayout new_layout);

	private:
		VulkanContext m_context = nullptr;

		VkImage m_image = nullptr;
		VkDeviceMemory m_memory = nullptr;
		VkImageView m_view = nullptr;

		VkSampler m_sampler = nullptr;
	};

	TextureInfo::Format vulkan_format_to_texture_format(VkFormat format);
	VkFormat texture_format_to_vulkan_format(TextureInfo::Format format);


	VkFilter sampler_filter_to_vulkan_filter(SamplerInfo::Filter filter);
	VkSamplerAddressMode sampler_wrapper_to_vulkan_adress_mode(SamplerInfo::Wrapper wrapper);
}