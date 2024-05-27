#pragma once

#include "Renderer/RendererAPI.h"

// Vulkan
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommand.h"
#include "VulkanTexture.h"

#include <vector>

namespace hit
{
    class VulkanRenderer : public RendererAPI
    {
    public:
        VulkanRenderer() = default;
        ~VulkanRenderer() = default;

        inline const VulkanDevice* get_device() const { return &m_device; }
        inline const VulkanSwapchain* get_swapchain() const { return &m_swapchain; }

        inline ui32 get_current_image_index() const { return m_current_image_index; }
        inline ui32 get_current_frame() const { return m_current_frame; }

        inline ui32 get_image_count() const { return m_swapchain.get_image_count(); }
        inline ui32 get_max_frames_in_flight() const { return m_swapchain.get_max_frames_in_flight(); }

        inline const VulkanCommand& get_graphics_command() const { return m_graphics_commands[m_current_image_index]; }

        inline Ref<VulkanTexture> get_placeholder_texture() const { return m_placeholder_texture; }

    public:
        void set_viewport(i32 x, i32 y, i32 width, i32 height);
        void set_scissor(i32 x, i32 y, i32 width, i32 height);

        ui32 get_swapchain_image_count() const override;
        const Ref<Texture> get_swapchain_image(ui32 index) const override;
        std::vector<Ref<Texture>> get_swapchain_images() const override;

        Ref<Texture> acquire_texture() override;
        Ref<Renderpass> acquire_renderpass() override;
        Ref<RenderPipeline> acquire_render_pipeline() override;
        Ref<Buffer> acquire_buffer() override;

    protected:
        bool initialize() override;
        void shutdown() override;

        bool begin_frame() override;
        bool end_frame() override;

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

        // default resources
        Ref<VulkanTexture> m_placeholder_texture;
    };
}