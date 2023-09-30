workspace "HitEngine"
    architecture "x64"

    configurations 
    { 
        "Debug",
        "Release"
    }

    filter "action:vs*"
        defines
        {
            "MSVC"
        }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "HIT_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        symbols "On"

        defines 
        {
            "_DEBUG",
            "HIT_DEBUG"
        }

        buildoptions 
        {
            "/MDd",
		}

    filter "configurations:Release"
        optimize "On"

        defines
        {
            "HIT_RELEASE"
        }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "HitEngine"
include "Runtime"
include "Test"