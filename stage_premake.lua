ROOT_PATH = "../../"

function conan(configs)
	for _, config in  pairs(configs) do
		include(ROOT_PATH.."build/deps/"..config.."/conandeps.premake5")
		conan_setup_build(string.lower(config).."_x86_64")
	end
end

function conan_exec(configs)
	for _, config in  pairs(configs) do
		filter{"configurations:"..config}
		include(ROOT_PATH.."build/deps/"..config.."/conandeps.premake5")
		conan_setup_link(string.lower(config).."_x86_64")
	end
	filter {}
end

function example_project(name,configs)
	project(name)
	kind "WindowedApp"
	language "C++"
	filter {}
	libdirs { ROOT_PATH.."lib/TijaNG/%{cfg.buildcfg}/" }
	links "TijaNG"
	targetdir (ROOT_PATH.."build/%{prj.name}/Release")
	includedirs (ROOT_PATH.."include")
	buildoptions { "/utf-8" }
	debugargs { _MAIN_SCRIPT_DIR .."/".. ROOT_PATH.."examples/data" }
	files { ROOT_PATH.."examples/" .. string.lower(name) .. ".cpp", ROOT_PATH.."examples/main.cpp" }
	defines { "TIJANG_EXAMPLE_NAME="..name }
	conan_exec(configs)
end

workspace "TijaNG"
	configuration_names = { "Debug", "Release", "RelWithDebInfo" }
	architecture "x64"
	location (ROOT_PATH.."build")
	cppdialect "c++20"
	startproject "00_Empty"
    platforms { "x86_64" }
	configurations(configuration_names)
	conan(configuration_names)
	
	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"

	filter "configurations:RelWithDebInfo"
		defines { "NDEBUG" }
		optimize "On"
		runtime "Release"
		symbols "On"
	filter {}

	example_project("00_Empty",configuration_names)
	example_project("01_Window",configuration_names)
	example_project("02_Triangle",configuration_names)