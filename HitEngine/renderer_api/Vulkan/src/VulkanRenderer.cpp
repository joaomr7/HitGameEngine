#include "VulkanRenderer.h"

#include "VulkanRenderpass.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "Platform/Platform.h"

namespace hit
{
    bool VulkanRenderer::initialize()
    {
        const auto& renderer_configuration = get_engine()->get_renderer_config();

        VulkanDeviceInfo device_info;
        device_info.engine = (Engine*)get_engine();
        device_info.window = (Window*)Platform::get_main_window();

        if(!m_device.initialize(device_info))
        {
            hit_fatal("Failed to initialize vulkan device!");
            return false;
        }

        VulkanSwapchainInfo swapchain_info;
        swapchain_info.engine = (Engine*)get_engine();
        swapchain_info.window = (Window*)Platform::get_main_window();

        if(!m_swapchain.initialize(this, swapchain_info))
        {
            hit_fatal("Failed to initialize vulkan swapchain!");
            return false;
        }

        // reset frame count every time swapchain is created
        m_current_frame = 0;

        if(!create_sync_objects())
        {
            hit_error("Failed to create vulkan syn objects!");
            return false;
        }

        if(!create_command_buffers())
        {
            hit_error("Failed to create vulkan command buffers!");
            return false;
        }

        return true;
    }

    void VulkanRenderer::shutdown()
    {
        m_device.wait_idle();

        m_graphics_commands.clear();

        destroy_sync_objects();

        m_swapchain.shutdown(&m_device);
        m_device.shutdown();
    }

    bool VulkanRenderer::begin_frame()
    {
        auto front_renderer = (Renderer*)get_frontend_renderer();

        // check if need to recreate swapchain
        if(front_renderer->m_frame_last_generation != front_renderer->m_frame_generation)
        {
            if(!recreate_swapchain())
            {
                hit_error("Failed to recreate swapchain!");
            }

            front_renderer->m_frame_last_generation = front_renderer->m_frame_generation;
            return false;
        }

        // wait current frame to complete
        auto result = vkWaitForFences(
            m_device.get_device(),
            1,
            &m_in_flight_fences[m_current_frame],
            true,
            UINT64_MAX);

        if(!check_vk_result(result))
        {
            hit_error("Failed to wait 'in_flight_fence'!");
            return false;
        }

        result = vkAcquireNextImageKHR(
            m_device.get_device(),
            m_swapchain.get_swapchain(),
            UINT64_MAX,
            m_available_image_semaphores[m_current_frame],
            nullptr,
            &m_current_image_index);

        if(result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            if(!recreate_swapchain())
            {
                hit_error("Failed to recreate swapchain!");
            }

            return false;
        }

        if(!check_vk_result(result) && result != VK_SUBOPTIMAL_KHR)
        {
            hit_error("Failed to acquire next swapchain image index!");
            return false;
        }

        auto& current_command = m_graphics_commands[m_current_image_index];
        if(!current_command.begin_command())
        {
            hit_error("Failed to begin graphics commands!");
            return false;
        }

        return true;
    }

    bool VulkanRenderer::end_frame()
    {
        auto& current_command = m_graphics_commands[m_current_image_index];
        current_command.end_command();
        auto current_command_buffer = current_command.get_command_buffer();

        // check if current image is not in use, and wait
        if(m_images_in_flight[m_current_image_index] != nullptr)
        {
            auto result = vkWaitForFences(
                m_device.get_device(),
                1,
                m_images_in_flight[m_current_image_index],
                true,
                UINT64_MAX);

            if(!check_vk_result(result))
            {
                hit_error("Failed to wait in flight image!");
                return false;
            }
        }

        // store current fence
        m_images_in_flight[m_current_image_index] = &m_in_flight_fences[m_current_frame];

        // reset current fence
        auto result = vkResetFences(m_device.get_device(), 1, &m_in_flight_fences[m_current_frame]);
        if(!check_vk_result(result))
        {
            hit_error("Failed to reset 'in_flight_fences'!");
            return false;
        }

        // submit command
        VkSubmitInfo submit_info{ };
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &current_command_buffer;

        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &m_available_image_semaphores[m_current_frame];
        
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &m_finished_render_semaphores[m_current_frame];

        VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.pWaitDstStageMask = wait_stages;

        result = m_device.submit_graphics_queue(&submit_info, m_in_flight_fences[m_current_frame]);
        if(!check_vk_result(result))
        {
            hit_error("Failed to submit commands to graphics queue!");
            return false;
        }

        // present current image
        VkPresentInfoKHR present_info{ };
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &m_finished_render_semaphores[m_current_frame];

        VkSwapchainKHR swapchain_handle = m_swapchain.get_swapchain();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain_handle;

        present_info.pImageIndices = &m_current_image_index;
        present_info.pResults = nullptr;

        result = m_device.submit_present_queue(&present_info);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            hit_info("Swapchain recreated because it's out of date or suboptimal.");
            recreate_swapchain();
            return true;
        }
        else if(!check_vk_result(result))
        {
            hit_error("Failed to submit present queue!");
            return false;
        }

        // update current frame
        m_current_frame = (m_current_frame + 1) % m_swapchain.get_max_frames_in_flight();

        return true;
    }

    ui32 VulkanRenderer::get_swapchain_image_count() const
    {
        return m_swapchain.get_image_count();
    }

    const Ref<Texture> VulkanRenderer::get_swapchain_image(ui32 index) const
    {
        return m_swapchain.get_image(index);
    }

    std::vector<Ref<Texture>> VulkanRenderer::get_swapchain_images() const
    {
        std::vector<Ref<Texture>> images;
        for(ui32 i = 0; i < get_swapchain_image_count(); i++)
        {
            images.push_back(get_swapchain_image(i));
        }

        return images;
    }

    Ref<Renderpass> VulkanRenderer::acquire_renderpass()
    {
        return create_ref<VulkanRenderpass>(this);
    }

    bool VulkanRenderer::recreate_swapchain()
    {
        auto result = m_device.wait_idle();
        if(!check_vk_result(result))
        {
            hit_error("Failed to wait vulkan device idle!");
            return false;
        }

        m_device.update_swapchain_support();

        VulkanSwapchainInfo swapchain_info;
        swapchain_info.engine = (Engine*)get_engine();
        swapchain_info.window = (Window*)Platform::get_main_window();

        if(!m_swapchain.recreate(this, swapchain_info))
        {
            hit_error("Failed to recreate vulkan swapchain!");
            return false;
        }

        // reset command buffers
        if(!create_command_buffers())
        {
            return false;
        }

        for(auto& in_flight : m_images_in_flight) in_flight = nullptr;

        m_current_frame = 0;

        return true;
    }

    bool VulkanRenderer::create_command_buffers()
    {
        if(m_graphics_commands.empty())
        {
            m_graphics_commands.resize(m_swapchain.get_image_count());
        }

        for(auto& command : m_graphics_commands)
        {
            if(command.get_command_buffer())
            {
                if(!command.reset())
                {
                    hit_error("Failed to reset command buffers!");
                    return false;
                }
            }
            else
            {
                if(!allocate_graphics_command(&m_device, false, command))
                {
                    hit_error("Failed to allocate command buffers!");
                    return false;
                }
            }
        }

        return true;
    }

    bool VulkanRenderer::create_sync_objects()
    {
        ui32 max_frames_in_flight = m_swapchain.get_max_frames_in_flight();

        m_available_image_semaphores.resize(max_frames_in_flight);
        m_finished_render_semaphores.resize(max_frames_in_flight);
        m_in_flight_fences.resize(max_frames_in_flight);

        VkSemaphoreCreateInfo semaphore_info{ };
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{ };
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(ui32 i = 0; i < max_frames_in_flight; i++)
        {
            VkResult result;

            result = vkCreateSemaphore(
                m_device.get_device(),
                &semaphore_info,
                m_device.get_alloc_callback(),
                &m_available_image_semaphores[i]);

            if(!check_vk_result(result))
            {
                hit_error("Failed to create 'available_image_semaphore'!");
                return false;
            }

            result = vkCreateSemaphore(
                m_device.get_device(),
                &semaphore_info,
                m_device.get_alloc_callback(),
                &m_finished_render_semaphores[i]);

            if(!check_vk_result(result))
            {
                hit_error("Failed to create 'finished_render_semaphore'!");
                return false;
            }

            result = vkCreateFence(
                m_device.get_device(),
                &fence_info,
                m_device.get_alloc_callback(),
                &m_in_flight_fences[i]);

            if(!check_vk_result(result))
            {
                hit_error("Failed to create 'frames_in_flight_fence'!");
                return false;
            }
        }

        m_images_in_flight.resize(m_swapchain.get_image_count());
        for(auto& in_flight : m_images_in_flight) in_flight = nullptr;

        return true;
    }

    void VulkanRenderer::destroy_sync_objects()
    {
        if(!m_available_image_semaphores.empty())
        {
            for(auto& semaphore : m_available_image_semaphores)
            {
                vkDestroySemaphore(m_device.get_device(), semaphore, m_device.get_alloc_callback());
            }
            m_available_image_semaphores.clear();
        }

        if(!m_finished_render_semaphores.empty())
        {
            for(auto& semaphore : m_finished_render_semaphores)
            {
                vkDestroySemaphore(m_device.get_device(), semaphore, m_device.get_alloc_callback());
            }
            m_finished_render_semaphores.clear();
        }

        if(!m_in_flight_fences.empty())
        {
            for(auto& fence : m_in_flight_fences)
            {
                vkDestroyFence(m_device.get_device(), fence, m_device.get_alloc_callback());
            }
            m_in_flight_fences.clear();
        }
    }
}