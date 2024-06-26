project "Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    characterset ("MBCS")

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    postbuildcommands
    {
        "{COPY} %{wks.location}/assets %{cfg.targetdir}/assets",
        "{COPY} %{wks.location}/assets %{wks.location}/Runtime/assets"
    }

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