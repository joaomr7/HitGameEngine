#pragma once

#include "vulkan/vulkan.h"

#define check_vk_result(result) ((result) == VK_SUCCESS)

namespace hit
{
	using VulkanContext = class VulkanRenderer*;
}