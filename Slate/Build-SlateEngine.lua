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

    "include/**.hpp",
    "include/**.hxx",
    "include/**.h",
  }

  includedirs
  {
    "src",
    "include"
  }

  libdirs
  {
    
  }

  links
  {
    "d3d11",
    "dxgi",
  }

  filter "system:windows"
    systemversion "latest"
    buildoptions { "/Zc:preprocessor" }
    defines
    {
      "NOMINMAX",
      "WIN32_LEAN_AND_MEAN",
    }

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
