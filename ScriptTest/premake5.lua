project "ScriptTest"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    characterset ("MBCS")

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "%{wks.location}/ScriptTest/src/**.h",
        "%{wks.location}/ScriptTest/src/**.cpp",
    }

    includedirs 
    {
        "%{wks.location}/HitEngine/include",
    }

    links
    {
        "HitEngine"
    }

    runtime_output_dir = "%{wks.location}/bin/" .. outputdir .. "/Runtime"
    postbuildcommands
    {
        "{COPY} %{cfg.buildtarget.relpath} %{runtime_output_dir}"
    }