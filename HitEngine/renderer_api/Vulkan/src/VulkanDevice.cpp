#include "VulkanDevice.h"

#include "Core/Engine.h"
#include "Core/Log.h"
#include "Platform/Window.h"

#include <vector>
#include <string.h>
#include <map>
#include <set>
#include <tuple>

namespace hit::vulkan_helper
{
#ifdef HIT_DEBUG_RENDER
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data
    )
    {
        switch(message_severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            {
                hit_info(p_callback_data->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            {
                hit_warning(p_callback_data->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            {
                hit_error(p_callback_data->pMessage);
                break;
            }
        }

        return VK_FALSE;
    };

    static VkResult create_debug_utils_messenger_ext(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
        const VkAllocationCallbacks* alloc_callback,
        VkDebugUtilsMessengerEXT* p_debug_messenger)
    {
        auto create_function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if(create_function)
        {
            return create_function(instance, p_create_info, alloc_callback, p_debug_messenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void destroy_debug_utils_messenger_ext(
        VkInstance instance, 
        VkDebugUtilsMessengerEXT debug_messenger, 
        const VkAllocationCallbacks* alloc_callback)
    {
        auto destroy_function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if(destroy_function)
        {
            destroy_function(instance, debug_messenger, alloc_callback);
        }
    }
#endif
    // Istance creation, helper functions
    static bool check_vulkan_instance_layer_support(const std::vector<const char*>& layers)
    {
        ui32 layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for(auto layer_name : layers)
        {
            bool found = false;

            for(const auto& layer : available_layers)
            {
                if(std::strcmp(layer_name, layer.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }

            if(!found) return false;
        }

        return true;
    }

    static std::vector<const char*> get_vulkan_required_extensions()
    {
        // get glfw extensions
        ui32 glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        // transform texensions to vector
        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    #ifdef HIT_DEBUG_RENDER
        // add debug utils extension
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

        return extensions;
    }

    // Device creation, helper functions
    static std::vector<PhysicalDeviceDetail> get_available_physical_devices_details(VkInstance instance)
    {
        std::vector<PhysicalDeviceDetail> details;

        ui32 devices_count;
        vkEnumeratePhysicalDevices(instance, &devices_count, nullptr);

        if(devices_count == 0) return { };

        std::vector<VkPhysicalDevice> physical_devices(devices_count);
        vkEnumeratePhysicalDevices(instance, &devices_count, physical_devices.data());

        details.reserve(devices_count);
        for(auto device : physical_devices)
        {
            PhysicalDeviceDetail detail;
            detail.device = device;

            vkGetPhysicalDeviceProperties(device, &detail.properties);
            vkGetPhysicalDeviceFeatures(device, &detail.features);

            details.push_back(detail);
        }

        return details;
    }

    static bool check_physical_devices_required_extension(VkPhysicalDevice device, const std::vector<const char*>& extensions)
    {
        ui32 extensions_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extensions_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, available_extensions.data());

        for(auto& extension : extensions)
        {
            bool found = false;

            for(auto& available_extension : available_extensions)
            {
                if(std::strcmp(extension, available_extension.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }

            if(!found) return false;
        }

        return true;
    }

    static std::vector<VkQueueFamilyProperties> get_devices_queue_family_properties(VkPhysicalDevice device)
    { 
        ui32 queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

        return queue_families;
    }

    static DeviceQueueIndices get_device_queue_indices(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<VkQueueFamilyProperties>& queue_properties)
    {
        DeviceQueueIndices indices;

        for(i32 i = 0; auto& queue_property : queue_properties)
        {
            if(queue_property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics_index = i;
            }

            //check present queue support
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, (ui32)i, surface, &present_support);

                if(present_support)
                {
                    indices.present_index = i;
                }
            }

            if(queue_property.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                indices.compute_index = i;
            }

            if(queue_property.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices.transfer_index = i;
            }

            i++;
        }

        return indices;
    }

    static std::multimap<ui16, PhysicalDeviceDetail> rate_devices(
        const std::vector<PhysicalDeviceDetail>& devices_details, 
        const std::vector<const char*>& required_extensions,
        VkSurfaceKHR surface)
    {
        std::multimap<ui16, PhysicalDeviceDetail> rate;

        for(auto& detail : devices_details)
        {
            ui16 score = 0;

            if(detail.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 1;
            }

            if(detail.features.samplerAnisotropy == VK_TRUE)
            {
                score++;
            }
            else
            {
                // feature requested
                continue;
            }

            // check queue families
            ui16 required_queues_count = 0;
            auto queue_properties = get_devices_queue_family_properties(detail.device);
            auto queue_indices = get_device_queue_indices(detail.device, surface, queue_properties);

            if(!check_physical_devices_required_extension(detail.device, required_extensions))
            {
                continue;
            }

            // request all required queue families
            if(queue_indices.graphics_index == -1) continue;
            if(queue_indices.present_index == -1) continue;
            if(queue_indices.compute_index == -1) continue;
            if(queue_indices.transfer_index == -1) continue;

            // add just devices with more than 0 of score
            if(score > 0)
            {
                rate.insert(std::make_pair(score, detail));
            }
        }

        return rate;
    }

    // Detect depth format function
    static std::tuple<VkFormat, VkImageTiling, ui8> detect_device_depth_format(VkPhysicalDevice device)
    {
        // Format + depth channel size
        std::tuple<VkFormat, ui8> depth_candidates[3]
        {
            { VK_FORMAT_D32_SFLOAT,         4 },
            { VK_FORMAT_D32_SFLOAT_S8_UINT, 4 },
            { VK_FORMAT_D24_UNORM_S8_UINT,  3 }
        };

        VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        for(auto [format, channels] : depth_candidates)
        {
            VkFormatProperties format_properties;
            vkGetPhysicalDeviceFormatProperties(device, format, &format_properties);

            if((format_properties.optimalTilingFeatures & features) == features)
            {
                return { format, VK_IMAGE_TILING_OPTIMAL, channels };
            }
            else if((format_properties.linearTilingFeatures & features) == features)
            {
                return { format, VK_IMAGE_TILING_LINEAR, channels };
            }
        }

        return { VK_FORMAT_UNDEFINED, VK_IMAGE_TILING_OPTIMAL, 0 };
    }
}

namespace hit
{
    bool VulkanDevice::initialize(const VulkanDeviceInfo& info)
    {
        // create vulkan instance
        std::vector<const char*> instance_layers;

    #ifdef HIT_DEBUG_RENDER
        instance_layers.push_back("VK_LAYER_KHRONOS_validation");

        if(!vulkan_helper::check_vulkan_instance_layer_support(instance_layers))
        {
            hit_fatal("Vulkan instance do not support all required layers. Can't initialize vulkan device!");
            return false;
        }
    #endif

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = info.engine->get_game_name().c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // TODO: add custom app version
        app_info.pEngineName = "Hit Game Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0); // TODO: add custom engine version
        app_info.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo instance_info{};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;

        auto required_extensions = vulkan_helper::get_vulkan_required_extensions();
        instance_info.enabledExtensionCount = (ui32)required_extensions.size();
        instance_info.ppEnabledExtensionNames = required_extensions.data();

        instance_info.enabledLayerCount = (ui32)instance_layers.size();
        instance_info.ppEnabledLayerNames = instance_layers.size() > 0 ? instance_layers.data() : nullptr;

    #ifdef HIT_DEBUG_RENDER
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = vulkan_helper::vulkan_debug_callback;

        instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    #endif

        auto instance_creation_result = vkCreateInstance(&instance_info, m_allocation_callback, &m_instance);
        if(!check_vk_result(instance_creation_result))
        {
            hit_error("Failed to create vulkan instance. Can't initialize vulkan device!");
            return false;
        }

    #ifdef HIT_DEBUG_RENDER
        auto debug_creation_result = vulkan_helper::create_debug_utils_messenger_ext(m_instance, &debug_create_info, m_allocation_callback, &m_debug_messenger);
        if(!check_vk_result(debug_creation_result))
        {
            hit_error("Failed to create vulkan debug messenger. Can't initialize vulkan device!");
            return false;
        }
    #endif

        // create vulkan surface
        if(!check_vk_result(
            glfwCreateWindowSurface(m_instance, (GLFWwindow*)info.window->get_handle(), m_allocation_callback, &m_surface)
        ))
        {
            hit_error("Failed to create vulkan surface. Can't initialize vulkan device!");
            return false;
        }

        std::vector<const char*> required_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        // select vulkan physical device
        {
            auto available_devices = vulkan_helper::get_available_physical_devices_details(m_instance);
            if(available_devices.empty())
            {
                hit_error("No available physical devices. Can't initialize vulkan device!");
                return false;
            }

            auto rated_devices = vulkan_helper::rate_devices(available_devices, required_device_extensions, m_surface);

            if(rated_devices.empty())
            {
                hit_error("No suitable devices found. Can't initialize vulkan device!");
                return false;
            }

            // select the highest rated device
            m_device_details = rated_devices.rbegin()->second;

            // get swapchain support details
            update_swapchain_support();

            ui32 swapchain_formats_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_device_details.device, m_surface, &swapchain_formats_count, nullptr);

            if(swapchain_formats_count > 0)
            {
                m_device_swapchain_support_details.formats.resize(swapchain_formats_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    m_device_details.device,
                    m_surface,
                    &swapchain_formats_count,
                    m_device_swapchain_support_details.formats.data());
            }
            else
            {
                hit_error("Swapchain has no valid format. Can't initialize vulkan device!");
                return false;
            }

            ui32 swapchain_present_mode_counts;
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_device_details.device, m_surface, &swapchain_present_mode_counts, nullptr);

            if(swapchain_formats_count > 0)
            {
                m_device_swapchain_support_details.present_mode.resize(swapchain_formats_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    m_device_details.device,
                    m_surface,
                    &swapchain_formats_count,
                    m_device_swapchain_support_details.present_mode.data());
            }
            else
            {
                hit_error("Swapchain has no valid present mode. Can't initialize vulkan device!");
                return false;
            }

            // get depth format support
            std::tie(m_depth_format, m_depth_tiling, m_depth_channel_count) = 
                vulkan_helper::detect_device_depth_format(m_device_details.device);

            if(m_depth_format == VK_FORMAT_UNDEFINED)
            {
                hit_error("Failed to detect a device depth format. Can't initialize vulkan device!");
                return false;
            }
        }

        // create queue create infos
        auto queue_properties = vulkan_helper::get_devices_queue_family_properties(m_device_details.device);
        auto queue_indices = vulkan_helper::get_device_queue_indices(m_device_details.device, m_surface, queue_properties);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        f32 queue_prority = 1.0f;

        std::set<ui32> unique_index({
            (ui32)queue_indices.graphics_index,
            (ui32)queue_indices.present_index,
            (ui32)queue_indices.compute_index,
            (ui32)queue_indices.transfer_index });

        for(auto index : unique_index)
        {
            VkDeviceQueueCreateInfo queue_info{};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = index;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &queue_prority;

            queue_create_infos.push_back(queue_info);
        }

        // select device features
        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;

        // create device
        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = (ui32)queue_create_infos.size();
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.pEnabledFeatures = &device_features;
        device_create_info.enabledExtensionCount = (ui32)required_device_extensions.size();
        device_create_info.ppEnabledExtensionNames = required_device_extensions.data();

    #ifdef HIT_DEBUG_RENDER
        device_create_info.enabledLayerCount = (ui32)instance_layers.size();
        device_create_info.ppEnabledLayerNames = instance_layers.data();
    #endif

        VkResult device_creation_result = vkCreateDevice(
            m_device_details.device, 
            &device_create_info, 
            m_allocation_callback, 
            &m_device);

        if(!check_vk_result(device_creation_result))
        {
            hit_error("Failed to create vulkan device. Can't initialize vulkan device!");
            return false;
        }

        // get device queues
        vkGetDeviceQueue(m_device, (ui32)queue_indices.graphics_index, 0, &m_graphics_queue);
        vkGetDeviceQueue(m_device, (ui32)queue_indices.present_index, 0, &m_present_queue);
        vkGetDeviceQueue(m_device, (ui32)queue_indices.compute_index, 0, &m_compute_queue);
        vkGetDeviceQueue(m_device, (ui32)queue_indices.transfer_index, 0, &m_transfer_queue);

        m_graphics_queue_index = queue_indices.graphics_index;
        m_present_queue_index = queue_indices.present_index;
        m_compute_queue_index = queue_indices.compute_index;
        m_transfer_queue_index = queue_indices.transfer_index;

        // create command pools
        VkCommandPoolCreateInfo graphics_pool_info{ };
        graphics_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        graphics_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        graphics_pool_info.queueFamilyIndex = (ui32)queue_indices.graphics_index;

        if(!check_vk_result(vkCreateCommandPool(m_device, &graphics_pool_info, m_allocation_callback, &m_graphics_command_pool)))
        {
            hit_error("Failed to create vulkan graphics command pool!");
            return false;
        }

        if(queue_indices.graphics_index == queue_indices.present_index)
        {
            m_present_command_pool = m_graphics_command_pool;
        }
        else
        {
            VkCommandPoolCreateInfo present_pool_info{ };
            present_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            present_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            present_pool_info.queueFamilyIndex = (ui32)queue_indices.present_index;

            if(!check_vk_result(vkCreateCommandPool(m_device, &present_pool_info, m_allocation_callback, &m_present_command_pool)))
            {
                hit_error("Failed to create vulkan present command pool!");
                return false;
            }
        }

        VkCommandPoolCreateInfo compute_pool_info{ };
        compute_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        compute_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        compute_pool_info.queueFamilyIndex = (ui32)queue_indices.compute_index;

        if(!check_vk_result(vkCreateCommandPool(m_device, &compute_pool_info, m_allocation_callback, &m_compute_command_pool)))
        {
            hit_error("Failed to create vulkan compute command pool!");
            return false;
        }

        VkCommandPoolCreateInfo transfer_pool_info{ };
        transfer_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transfer_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        transfer_pool_info.queueFamilyIndex = (ui32)queue_indices.compute_index;

        if(!check_vk_result(vkCreateCommandPool(m_device, &transfer_pool_info, m_allocation_callback, &m_transfer_command_pool)))
        {
            hit_error("Failed to create vulkan transfer command pool!");
            return false;
        }

        return true;
    }

    void VulkanDevice::shutdown()
    {
        if(m_graphics_command_pool == m_present_command_pool)
        {
            if(m_graphics_command_pool)
            {
                vkDestroyCommandPool(m_device, m_graphics_command_pool, m_allocation_callback);
                m_graphics_command_pool = nullptr;
                m_present_command_pool = nullptr;
            }
        }
        else
        {
            if(m_graphics_command_pool)
            {
                vkDestroyCommandPool(m_device, m_graphics_command_pool, m_allocation_callback);
                m_graphics_command_pool = nullptr;
            }

            if(m_present_command_pool)
            {
                vkDestroyCommandPool(m_device, m_present_command_pool, m_allocation_callback);
                m_present_command_pool = nullptr;
            }
        }

        if(m_compute_command_pool)
        {
            vkDestroyCommandPool(m_device, m_compute_command_pool, m_allocation_callback);
            m_compute_command_pool = nullptr;
        }

        if(m_transfer_command_pool)
        {
            vkDestroyCommandPool(m_device, m_transfer_command_pool, m_allocation_callback);
            m_transfer_command_pool = nullptr;
        }

        if(m_device)
        {
            vkDestroyDevice(m_device, m_allocation_callback);
            m_device = nullptr;
        }

        if(m_surface)
        {
            vkDestroySurfaceKHR(m_instance, m_surface, m_allocation_callback);
            m_surface = nullptr;
        }

    #ifdef HIT_DEBUG_RENDER
        if(m_debug_messenger)
        {
            vulkan_helper::destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, m_allocation_callback);
            m_debug_messenger = nullptr;
        }
    #endif

        if(m_instance)
        {
            vkDestroyInstance(m_instance, m_allocation_callback);
            m_instance = nullptr;
        }
    }

    VkResult VulkanDevice::wait_idle() const
    { 
        return vkDeviceWaitIdle(m_device);
    }

    VkResult VulkanDevice::submit_graphics_queue(const VkSubmitInfo* submit_info, VkFence fence)
    { 
        return vkQueueSubmit(m_graphics_queue, 1, submit_info, fence);
    }

    VkResult VulkanDevice::submit_present_queue(const VkPresentInfoKHR* present_info)
    {
        return vkQueuePresentKHR(m_present_queue, present_info);
    }

    VkResult VulkanDevice::submit_compute_queue(const VkSubmitInfo* submit_info, VkFence fence)
    { 
        return vkQueueSubmit(m_compute_queue, 1, submit_info, fence);
    }

    VkResult VulkanDevice::submit_transfer_queue(const VkSubmitInfo* submit_info, VkFence fence)
    { 
        return vkQueueSubmit(m_transfer_queue, 1, submit_info, fence);
    }

    VkResult VulkanDevice::wait_graphics_queue() const
    { 
        return vkQueueWaitIdle(m_graphics_queue);
    }

    VkResult VulkanDevice::wait_present_queue() const
    { 
        return vkQueueWaitIdle(m_present_queue);
    }

    VkResult VulkanDevice::wait_compute_queue() const
    { 
        return vkQueueWaitIdle(m_compute_queue);
    }

    VkResult VulkanDevice::wait_transfer_queue() const
    { 
        return vkQueueWaitIdle(m_transfer_queue);
    }

    void VulkanDevice::update_swapchain_support()
    { 
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device_details.device, m_surface, &m_device_swapchain_support_details.capabilities);
    }

    DeviceQueueIndices VulkanDevice::get_device_queues_indices() const
    {
        auto queue_properties = vulkan_helper::get_devices_queue_family_properties(m_device_details.device);
        return vulkan_helper::get_device_queue_indices(m_device_details.device, m_surface, queue_properties);
    }

    i32 VulkanDevice::get_memory_type(ui32 type_filter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(m_device_details.device, &memory_properties);

        for(ui32 i = 0; i < memory_properties.memoryTypeCount; i++)
        {
            if((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return (i32)i;
            }
        }

        return -1; //invalid value
    }
}