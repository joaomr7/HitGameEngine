#pragma once

#include "VulkanCommon.h"
#include "Renderer/Buffer.h"

namespace hit
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(const VulkanContext context) : m_context(context), m_is_bind(false) { }
		~VulkanBuffer() = default;

		VulkanBuffer(VulkanBuffer&& other) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

		bool create(ui64 initial_size, BufferType type, BufferAllocationType allocation) override;
		void destroy() override;

		bool bind(ui64 offset = 0) override;
		bool unbind() override;

		bool resize(ui64 new_size) override;

		void* map_memory(ui64 offset, ui64 size) override;
		void unmap_memory(ui64 offset, ui64 size) override;

		bool flush(ui64 offset, ui64 size) override;

		bool read(ui64 offset, ui64 size, void** out_memory) override;
		bool load(ui64 offset, ui64 size, void* data) override;

		bool draw(ui64 offset, ui32 elem_count, bool bind_only) override;

		bool copy(Buffer& other, ui64 size, ui64 dest_offset = 0, ui64 src_offset = 0) override;

		inline const VkBuffer get_buffer() const { return m_buffer; }

	private:
		bool is_device_local();
		bool is_host_visible();
		bool is_host_coeherent();

	private:
		VulkanContext m_context;

		VkDeviceMemory m_memory;
		VkMemoryRequirements m_memory_requirements;
		ui32 m_memory_flags;
		i32 m_memory_type;

		VkBuffer m_buffer;
		ui32 m_buffer_flags;

		bool m_is_bind;
	};
}