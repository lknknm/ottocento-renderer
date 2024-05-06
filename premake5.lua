-- premake5.lua
workspace "OttocentoRenderer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "OttocentoRenderer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "OttocentoRenderer"