workspace "HitEngine"
    architecture "x64"

    configurations 
    { 
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "HitEngine"
include "Runtime"