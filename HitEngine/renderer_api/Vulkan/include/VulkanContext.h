#pragma once

#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

namespace hit
{
    struct VulkanContext
    {
        const VulkanDevice& device;
        const VulkanSwapchain& swapchain;
        const ui32& current_image_index;
        const ui32& m_current_frame;
    };
}