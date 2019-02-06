build_dir = "build\\"
bin_dir = build_dir .. "bin\\%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
bin_int_dir = build_dir .. "bin-int\\%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

j_app_path = "D:\\Dev\\CPP\\JApp"
j_app = require(j_app_path .. "\\util\\j_app")

workspace "UltimateWallpaper-2_0"
	architecture "x64"
	characterset ("MBCS")
	startproject "UltimateWallpaper-2_0"
	defines {
		"GLM_ENABLE_EXPERIMENTAL",
	}

j_app.includeJApp(j_app_path)

project "UltimateWallpaper"
	location "UltimateWallpaper"
	language "C++"
	cppdialect "C++17"
	systemversion "latest"

	filter "configurations:Debug"
		targetsuffix ("-dbg")
		kind "ConsoleApp"
		defines {
			"WP_DEBUG"
		}

	filter "configurations:Release"
		targetsuffix ("-rls")
		kind "ConsoleApp"
		defines {
			"WP_RELEASE"
		}

	filter "configurations:Dist"
		kind "WindowedApp"
		entrypoint "mainCRTStartup"
		defines {
			"WP_DIST"
		}

	filter {}

	targetdir("%{wks.location}\\" ..bin_dir)
	objdir("%{wks.location}\\" .. bin_int_dir)

	includedirs {
		"%{prj.name}\\src",
		"%{prj.name}\\src\\vendor",
		"%{wks.location}\\dependencies\\BOOST\\",
		"%{wks.location}\\dependencies\\BASS\\include\\",
	}

    libdirs {
        "%{wks.location}\\dependencies\\BASS\\lib\\",
    }

    links {
        "bass.lib",
        "basswasapi.lib"
    }

	files {
		"%{prj.name}\\*cfg.xml*",
		"%{prj.name}\\src\\**",
		"%{prj.name}\\resources\\**",
	}

	postbuildcommands {
		"{COPY} %{prj.location}resources %{wks.location}" .. bin_dir .. "\\resources",
		"copy %{prj.location}cfg.xml %{wks.location}" .. bin_dir .. "\\cfg.xml",
        "{COPY} %{wks.location}dependencies\\BASS\\dll %{wks.location}" .. bin_dir
	}

j_app.configureProject(j_app_path, "UltimateWallpaper-2_0", "UltimateWallpaper")
