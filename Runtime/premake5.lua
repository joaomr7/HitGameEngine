project "Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    characterset ("MBCS")

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        --PhysWorld
        "%{wks.location}/Runtime/src/**.h",
        "%{wks.location}/Runtime/src/**.cpp",
    }

    includedirs 
    {
        "%{wks.location}/HitEngine/include",
    }

    links
    {
        "HitEngine"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

        defines 
        {
            "_DEBUG"
        }

        buildoptions 
        {
            "/MDd",
		}

    filter "configurations:Release"
        optimize "On"