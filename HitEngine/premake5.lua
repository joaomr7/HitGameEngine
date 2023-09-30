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
        "%{wks.location}/HitEngine/include/**.h",
        "%{wks.location}/HitEngine/src/**.cpp",
    }

    includedirs 
    {
        "%{wks.location}/HitEngine/include",
    }