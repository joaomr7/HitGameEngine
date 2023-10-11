#pragma once

#include "Renderer/RendererAPI.h"

// Vulkan
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanContext.h"
#include "VulkanCommand.h"

#include <vector>

namespace hit
{
    class VulkanRenderer : public RendererAPI
    {
    public:
        bool initialize() override;
        void shutdown() override;

        bool begin_frame() override;
        bool end_frame() override;

        inline const VulkanContext& get_context() const 
        {
            return
            {
                m_device,
                m_swapchain,
                m_current_image_index,
                m_current_frame
            };
        }

    private:
        bool recreate_swapchain();

        bool create_command_buffers();

        bool create_sync_objects();
        void destroy_sync_objects();

    private:
        VulkanDevice m_device;
        VulkanSwapchain m_swapchain;

        ui32 m_current_image_index;
        ui32 m_current_frame;

        // command buffers -> one per image count
        std::vector<VulkanCommand> m_graphics_commands;

        // frame sync objects -> one per images in flight
        std::vector<VkSemaphore> m_available_image_semaphores;
        std::vector<VkSemaphore> m_finished_render_semaphores;
        std::vector<VkFence> m_in_flight_fences;

        std::vector<VkFence*> m_images_in_flight;
    };
}