include "Dependencies.lua"

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

group "Dependencies"
include "Dependencies/GLFW"
group ""

group "Script"
include "ScriptTest"
group ""

include "HitEngine"
include "Runtime"
include "Test"