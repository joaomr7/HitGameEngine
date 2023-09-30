project "Test"
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
        "%{wks.location}/Test/src/**.h",
        "%{wks.location}/Test/src/**.cpp",
    }

    includedirs 
    {
        "%{wks.location}/HitEngine/include",
    }

    links
    {
        "HitEngine"
    }