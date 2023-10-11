-- Hit Engine Dependencies
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include Directories
include_dir = {}
include_dir["GLFW"] = "%{wks.location}/Dependencies/GLFW/include"
include_dir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

lib_dir = {}
lib_dir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

libs = {}
libs["Vulkan"] = "%{lib_dir.VulkanSDK}/vulkan-1.lib"
