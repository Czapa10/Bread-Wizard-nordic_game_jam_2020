workspace "NordicGameJam"

architecture "x64"
location "../"

configurations{
    "Debug",
    "Release",
}

project "NordicGameJam"
    location "../"
    language "C++"
    cppdialect "C++17"
	kind "ConsoleApp"

    targetdir "../bin/bin"
    objdir "../bin/obj"

    includedirs{
        "../src",
		"../vendor/glfw/include",
		"../vendor/glew/include",
		"../vendor/imgui",
		"../vendor/FastMath",
		"../vendor/irrklang/include"
    }
   
    libdirs{
		"../vendor/glfw/lib-vc2019",
		"../vendor/glew/lib/Release/x64",
		"../vendor/irrklang/lib"
    }

    links{
        "glfw3.lib",
        "glew32s.lib",
        "opengl32.lib",
        "irrKlang.lib"
    }

	defines "GLEW_STATIC"
	
    files{
        "../src/main.cpp",
		"../vendor/imgui/**.h",
		"../vendor/imgui/**.cpp"
    }

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
