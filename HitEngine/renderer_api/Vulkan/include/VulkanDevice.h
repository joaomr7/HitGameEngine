#pragma once

#include "Core/Types.h"
#include "VulkanCommon.h"

#include <vector>

namespace hit
{
    struct VulkanDeviceInfo
    {
        class Engine* engine;
        class Window* window;
    };

    struct PhysicalDeviceDetail
    {
        VkPhysicalDevice device;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
    };

    struct DeviceQueueIndices
    {
        i32 graphics_index = -1;
        i32 present_index = -1;
        i32 compute_index = -1;
        i32 transfer_index = -1;
    };

    struct SwapchainDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_mode;
    };

    // Vulkan device interface
    class VulkanDevice final
    {
    public:
        VulkanDevice() = default;
        ~VulkanDevice() = default;

        bool initialize(const VulkanDeviceInfo& info);
        void shutdown();

        VkResult wait_idle() const;

        VkResult submit_graphics_queue(const VkSubmitInfo* submit_info, VkFence fence = nullptr);
        VkResult submit_present_queue(const VkPresentInfoKHR* present_info);
        VkResult submit_compute_queue(const VkSubmitInfo* submit_info, VkFence fence = nullptr);
        VkResult submit_transfer_queue(const VkSubmitInfo* submit_info, VkFence fence = nullptr);

        VkResult wait_graphics_queue() const;
        VkResult wait_present_queue() const;
        VkResult wait_compute_queue() const;
        VkResult wait_transfer_queue() const;

        void update_swapchain_support();

    public:
        inline const VkAllocationCallbacks* get_alloc_callback() const { return m_allocation_callback; }

        inline const VkInstance get_instance() const { return m_instance; }
        inline const VkSurfaceKHR get_surface() const { return m_surface; }

        inline const VkDevice get_device() const { return m_device; }
        inline const PhysicalDeviceDetail& get_device_details() const { return m_device_details; }

        inline const SwapchainDetails& get_swapchain_support_details() const { return m_device_swapchain_support_details; }

        inline const VkQueue get_graphics_queue() const { return m_graphics_queue; }
        inline const VkQueue get_present_queue() const { return m_present_queue; }
        inline const VkQueue get_compute_queue() const { return m_compute_queue; }
        inline const VkQueue get_transfer_queue() const { return m_transfer_queue; }

        inline const ui32 get_graphics_queue_index() const { return m_graphics_queue_index; }
        inline const ui32 get_present_queue_index() const { return m_present_queue_index; }
        inline const ui32 get_compute_queue_index() const { return m_compute_queue_index; }
        inline const ui32 get_transfer_queue_index() const { return m_transfer_queue_index; }

        inline const VkCommandPool get_graphics_command_pool() const { return m_graphics_command_pool; }
        inline const VkCommandPool get_present_command_pool() const { return m_present_command_pool; }
        inline const VkCommandPool get_compute_command_pool() const { return m_compute_command_pool; }
        inline const VkCommandPool get_transfer_command_pool() const { return m_transfer_command_pool; }

        inline VkFormat get_depth_format() const { return m_depth_format; }
        inline VkImageTiling get_depth_tiling() const { return m_depth_tiling; }
        inline ui8 get_depth_channel_count() const { return m_depth_channel_count; }

        DeviceQueueIndices get_device_queues_indices() const;
        i32 get_memory_type(ui32 type_filter, VkMemoryPropertyFlags properties) const;

    private:
        // instance and surface
        VkInstance m_instance = nullptr;
        VkSurfaceKHR m_surface = nullptr;

        // TODO: add custom allocation callback
        VkAllocationCallbacks* m_allocation_callback = nullptr;

    #ifdef HIT_DEBUG_RENDER
        // Vulkan validation layer
        VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
    #endif

        // device
        VkDevice m_device = nullptr;
        PhysicalDeviceDetail m_device_details;

        SwapchainDetails m_device_swapchain_support_details;

        // vulkan queues
        VkQueue m_graphics_queue = nullptr;
        ui32 m_graphics_queue_index;

        VkQueue m_present_queue = nullptr;
        ui32 m_present_queue_index;

        VkQueue m_compute_queue = nullptr;
        ui32 m_compute_queue_index;

        VkQueue m_transfer_queue = nullptr;
        ui32 m_transfer_queue_index;

        // command pools
        VkCommandPool m_graphics_command_pool = nullptr;
        VkCommandPool m_present_command_pool = nullptr;
        VkCommandPool m_compute_command_pool = nullptr;
        VkCommandPool m_transfer_command_pool = nullptr;

        // supported depth format
        VkFormat m_depth_format;
        VkImageTiling m_depth_tiling;
        ui8 m_depth_channel_count;
    };
}