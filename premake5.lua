workspace "tm3d-dx11"
architecture "x64"
startproject "tm3d-dx11"

configurations {
   "Debug",
   "Release",
}

project "tm3d-dx11"
location "tm3d"
kind "ConsoleApp"
language "c++"

targetdir ("run_tree/")
objdir ("run_tree/obj/%{prj.name}/%{cfg.buildcfg}/")

files {
   "src/**.h",
   "src/**.cpp",
}

includedirs {
   "external/include",
}

libdirs {
   "external/lib"
}

links {
   "opengl32.lib"
}

filter "system:windows"
cppdialect "C++20"
staticruntime "on"
systemversion "latest"

buildoptions "-Zc:strictStrings-"

defines {
   "OS_WIN32",
}

filter "configurations:Debug"
defines "DEBUG"
defines "_DEBUG"
symbols "on"

filter "configurations:Release"
defines "RELEASE"
defines "NDEBUG"
optimize "on"
