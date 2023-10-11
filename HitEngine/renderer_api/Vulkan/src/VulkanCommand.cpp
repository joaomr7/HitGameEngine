#include "VulkanCommand.h"

#include "Core/Log.h"

namespace hit
{
	bool VulkanCommand::allocate_command(const VulkanCommandInfo& info)
	{
		m_command_info = info;

		VkCommandPool command_pool = nullptr;

		//select command pool
		switch(m_command_info.queue)
		{
			case VulkanCommandInfo::QueueGraphics:	command_pool = m_command_info.device->get_graphics_command_pool(); break;
			case VulkanCommandInfo::QueueCompute:	command_pool = m_command_info.device->get_compute_command_pool(); break;
			case VulkanCommandInfo::QueueTransfer:	command_pool = m_command_info.device->get_transfer_command_pool(); break;
		}

		// allocate command pool
		VkCommandBufferAllocateInfo allocate_info{ };
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.commandPool = command_pool;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandBufferCount = 1;

		auto allocation_result = vkAllocateCommandBuffers(m_command_info.device->get_device(), &allocate_info, &m_command_buffer);

		if(!check_vk_result(allocation_result))
		{
			hit_error("Failed to allocate vulkan command buffer.");
			return false;
		}

		return true;
	}

	void VulkanCommand::deallocate_command()
	{
		hit_warning_if(!m_command_buffer, "Attempting to deallocate a not allocated vulkan command buffer!");

		if(m_command_buffer)
		{
			VkCommandPool command_pool = nullptr;

			//select command pool
			switch(m_command_info.queue)
			{
				case VulkanCommandInfo::QueueGraphics:	command_pool = m_command_info.device->get_graphics_command_pool(); break;
				case VulkanCommandInfo::QueueCompute:	command_pool = m_command_info.device->get_compute_command_pool(); break;
				case VulkanCommandInfo::QueueTransfer:	command_pool = m_command_info.device->get_transfer_command_pool(); break;
			}

			vkFreeCommandBuffers(m_command_info.device->get_device(), command_pool, 1, &m_command_buffer);
			m_command_buffer = nullptr;
		}
	}

	bool VulkanCommand::reset()
	{
		deallocate_command();
		return allocate_command(m_command_info);
	}

	bool VulkanCommand::begin_command()
	{
		VkCommandBufferBeginInfo begin_info{ };
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		if(m_command_info.single_use)
		{
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}

		if(!check_vk_result(vkBeginCommandBuffer(m_command_buffer, &begin_info)))
		{
			hit_error("Failed to begin vulkan command.");
			return false;
		}

		return true;
	}

	void VulkanCommand::end_command()
	{ 
		vkEndCommandBuffer(m_command_buffer);
	}

	bool allocate_graphics_command(VulkanDevice* device, bool single_use, VulkanCommand& command)
	{
		VulkanCommandInfo info;
		info.device = device;
		info.queue = VulkanCommandInfo::QueueGraphics;
		info.single_use = single_use;

		if(!command.allocate_command(info))
		{
			return false;
		}

		return true;
	}

	bool allocate_compute_command(VulkanDevice* device, bool single_use, VulkanCommand& command)
	{
		VulkanCommandInfo info;
		info.device = device;
		info.queue = VulkanCommandInfo::QueueCompute;
		info.single_use = single_use;

		if(!command.allocate_command(info))
		{
			return false;
		}

		return true;
	}

	bool allocate_transfer_command(VulkanDevice* device, bool single_use, VulkanCommand& command)
	{
		VulkanCommandInfo info;
		info.device = device;
		info.queue = VulkanCommandInfo::QueueTransfer;
		info.single_use = single_use;

		if(!command.allocate_command(info))
		{
			return false;
		}

		return true;
	}

	bool run_single_command(VulkanDevice* device, VulkanCommandInfo::QueueType queue, const std::function<void(VkCommandBuffer)>& fun)
	{
		VulkanCommand command;
		VulkanCommandInfo info;
		info.queue = queue;
		info.device = device;
		info.single_use = true;

		if(!command.allocate_command(info))
		{
			return false;
		}

		if(!command.begin_command())
		{
			return false;
		}

		fun(command.get_command_buffer());

		command.end_command();

		VkCommandBuffer command_buffer = command.get_command_buffer();

		//submit command buffer
		VkSubmitInfo submit_info{ };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		// TODO: check errors, warnign if queue is present
		switch(queue)
		{
			case VulkanCommandInfo::QueueGraphics:
			{
				device->submit_graphics_queue(&submit_info, nullptr);
				device->wait_graphics_queue();
				break;
			}

			case VulkanCommandInfo::QueueCompute:
			{
				device->submit_compute_queue(&submit_info, nullptr);
				device->wait_compute_queue();
				break;
			}

			case VulkanCommandInfo::QueueTransfer:
			{
				device->submit_transfer_queue(&submit_info, nullptr);
				device->wait_transfer_queue();
				break;
			}
		}

		command.deallocate_command();

		return true;
	}

	bool run_single_graphics_command(VulkanDevice* device, const std::function<void(VkCommandBuffer)>& fun)
	{
		return run_single_command(device, VulkanCommandInfo::QueueGraphics, fun);
	}

	bool run_single_compute_command(VulkanDevice* device, const std::function<void(VkCommandBuffer)>& fun)
	{
		return run_single_command(device, VulkanCommandInfo::QueueCompute, fun);
	}

	bool run_single_transfer_command(VulkanDevice* device, const std::function<void(VkCommandBuffer)>& fun)
	{
		return run_single_command(device, VulkanCommandInfo::QueueTransfer, fun);
	}
}