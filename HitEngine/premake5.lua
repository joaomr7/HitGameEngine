project "HitEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    characterset ("MBCS")

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        -- Engine code
        "%{wks.location}/HitEngine/include/**.h",
        "%{wks.location}/HitEngine/src/**.cpp",

        -- Renderer API's code
        "%{wks.location}/HitEngine/renderer_api/**.h",
        "%{wks.location}/HitEngine/renderer_api/**.cpp",
    }

    includedirs 
    {
        "%{wks.location}/HitEngine/include",
        "%{wks.location}/HitEngine/renderer_api/Vulkan/include",
        "%{include_dir.GLFW}",
        "%{include_dir.VulkanSDK}",
    }

    links
    {
        "GLFW",
        "%{libs.Vulkan}"
    }

    filter "system:windows"
        files
        {
            "%{wks.location}/HitEngine/win64/**.cpp"
        }