project "HitTools"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    characterset ("MBCS")

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "%{wks.location}/HitTools/include/**.h",
        "%{wks.location}/HitTools/vendor/stb_image/**.h",
        "%{wks.location}/HitTools/src/**.cpp",
        "%{wks.location}/HitTools/vendor/stb_image/**.cpp",
    }

    includedirs 
    {
        "%{wks.location}/HitEngine/include",
        "%{wks.location}/HitTools/include",
        "%{include_dir.VulkanSDK}",
    }

    links
    {
        "HitEngine"
    }

    postbuildcommands
    {
        "{COPY} %{cfg.buildtarget.relpath} %{wks.location}/script"
    }

    filter "configurations:Debug"
        symbols "On"

        defines 
        {
            "_DEBUG",
            "HIT_DEBUG",
            "HIT_DEBUG_RENDER"
        }

        buildoptions 
        {
            "/MDd",
        }

        links
        {
            "%{libs.ShaderC_debug}",
			"%{libs.SPIRV_Cross_debug}",
			"%{libs.SPIRV_Cross_GLSL_debug}"
        }

    filter "configurations:Release"
        optimize "On"

        defines
        {
            "HIT_RELEASE",
            "HIT_DEBUG_RENDER"
        }

        links
        {
            "%{libs.ShaderC_release}",
			"%{libs.SPIRV_Cross_release}",
			"%{libs.SPIRV_Cross_GLSL_release}"
        }