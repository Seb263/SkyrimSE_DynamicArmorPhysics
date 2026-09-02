-- include subprojects
includes("extern/CommonLibSSE-NG")

local sharedRoot = path.join(os.projectdir(), "..", "_XMakeShared")
target("commonlibsse-ng")
    set_targetdir(path.join(sharedRoot, "CommonLibSSE-NG", "windows", "x64"))
    set_objectdir(path.join(sharedRoot, "CommonLibSSE-NG", ".objs"))
    set_dependir(path.join(sharedRoot, "CommonLibSSE-NG", ".deps"))
target_end()

-- rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- packages required
add_requires("fmt", "simpleini", "magic_enum")

local projectRoot = os.projectdir():gsub("/", "\\") .. "\\"

-- project infos
local projectName = "DynamicArmorPhysics"
set_project(projectName)
set_version("1.0.4")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

target(projectName)
	set_symbols("debug")
	add_rules("commonlibsse-ng.plugin", {
		name = projectName,
		author = "Seb263",
		description = "Dynamic Armor Physics binaries"
	})

	-- packages to add
	add_packages("fmt", "simpleini", "magic_enum")
	add_includedirs("src")
	add_headerfiles("src/**.h", "src/**.hpp")
	add_files("src/**.cpp")
	set_pcxxheader("include/PCH.h")

	add_rules("utils.bin2c", {
		extensions = { ".psc", ".pex" }
	})
    add_files("extern/Papyrus/DynamicArmorPhysics.psc")
    add_files("extern/Papyrus/DynamicArmorPhysics.pex")

	-- release mode
	if is_mode("releasedbg") then
		set_optimize("smallest")
		add_cxxflags(
			"/Zo",
			"/Oy-",
			"/d1trimfile:" .. projectRoot,
			{ force = true }
		)
		add_ldflags("/DEBUG:FULL", "/OPT:REF", "/OPT:ICF", { force = true })
		
	-- debug mode
	elseif is_mode("debug") then
		set_optimize("none")
		add_cxxflags(
			"/RTC1",
			"/d1trimfile:" .. projectRoot,
			{ force = true }
		)
		add_ldflags("/DEBUG:FULL", { force = true })
	end
target_end()