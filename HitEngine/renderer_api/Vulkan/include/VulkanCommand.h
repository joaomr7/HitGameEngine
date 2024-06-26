#pragma once

#include "VulkanCommon.h"
#include "VulkanDevice.h"

#include <functional>

namespace hit
{
	struct VulkanCommandInfo
	{
		enum QueueType : ui8
		{
			QueueGraphics,
			QueueCompute,
			QueueTransfer
		};

		VulkanDevice* device;
		QueueType queue;
		bool single_use;
	};

	class VulkanCommand
	{
	public:
		VulkanCommand() = default;
		~VulkanCommand() = default;

		bool allocate_command(const VulkanCommandInfo& info);
		void deallocate_command();

		bool reset();

		bool begin_command();
		void end_command();

		inline const VkCommandBuffer get_command_buffer() const { return m_command_buffer; }

	private:
		VkCommandBuffer m_command_buffer;
		VulkanCommandInfo m_command_info;
	};

	bool allocate_graphics_command(VulkanDevice* device, bool single_use, VulkanCommand& command);
	bool allocate_compute_command(VulkanDevice* device, bool single_use, VulkanCommand& command);
	bool allocate_transfer_command(VulkanDevice* device, bool single_use, VulkanCommand& command);

	bool run_single_command(VulkanDevice* device, VulkanCommandInfo::QueueType queue, const std::function<void(VkCommandBuffer)>& fun);

	bool run_single_graphics_command(VulkanDevice* device, const std::function<void(VkCommandBuffer)>& fun);
	bool run_single_compute_command(VulkanDevice* device, const std::function<void(VkCommandBuffer)>& fun);
	bool run_single_transfer_command(VulkanDevice* device, const std::function<void(VkCommandBuffer)>& fun);
}