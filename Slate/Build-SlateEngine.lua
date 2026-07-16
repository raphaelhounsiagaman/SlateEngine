project "SlateEngine"
  kind "StaticLib"
  language "C++"
  cppdialect "C++23"

  targetdir ("../bin/%{cfg.buildcfg}")
  objdir ("../bin/Intermediates/%{cfg.builcfg}")

  files
  {
    "src/**.cpp",
    "src/**.cxx",
    "src/**.c",

    "src/**.hpp",
    "src/**.hxx",
    "src/**.h",

  }

  includedirs
  {
    "src",

  }

  libdirs
  {
    
  }

  filter "system:windows"
    systemversion "latest"

  filter "configurations:Debug"
    defines "SLATE_DEBUG"
    runtime "Debug"
    symbols "on"

  filter "configurations:Release"
    defines "SLATE_RELEASE"
    runtime "Release"
    optimize "on"

  filter "configurations:Dist"
    defines "SLATE_DIST"
    runtime "Release"
    optimize "full"
  filter{}
