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

-- spirv/shader_c debug libs
libs["ShaderC_debug"] = "%{lib_dir.VulkanSDK}/shaderc_sharedd.lib"
libs["SPIRV_Cross_debug"] = "%{lib_dir.VulkanSDK}/spirv-cross-cored.lib"
libs["SPIRV_Cross_GLSL_debug"] = "%{lib_dir.VulkanSDK}/spirv-cross-glsld.lib"

-- spirv/shader_c release libs
libs["ShaderC_release"] = "%{lib_dir.VulkanSDK}/shaderc_shared.lib"
libs["SPIRV_Cross_release"] = "%{lib_dir.VulkanSDK}/spirv-cross-core.lib"
libs["SPIRV_Cross_GLSL_release"] = "%{lib_dir.VulkanSDK}/spirv-cross-glsl.lib"