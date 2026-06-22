set_project ( "DVD Animation" )
set_version ( "1.0.0"   )
set_xmakever( "2.8.0"   )

-- GENERATE compile_commands.json -----------------------------------------------------
rule( "vscode.compile_commands" )
    after_config( function(_)
        import( "core.base.task" ).run( "project", {
            kind      = "compile_commands",
            outputdir = ".vscode"
        })
    end )
rule_end()


-- HELPERS ---------------------------------------------------------------------------
local function _config_proyect( target )
    local triple = import( "xmake.cfg_triple", { anonymous = true } )
    local flags  = import( "xmake.cfg_flags",  { anonymous = true } )
    local info   = triple.get( target )

    flags.apply      ( target, info )
    triple.print_info( target, info )
end


local function _run_process( target )
    import("core.base.process")

    local program = target:targetfile()
    local args    = target:get("runargs") or {}

    cprint( "${bright green}[Running: " .. program .. "]" )

    local proc = process.openv( program, args, { detach = true })
    local ok, status = proc:wait()

    if ok < 0 then
        cprint("${bright red}[Process failed with status: " .. ok .. "]")
    else
        if status == 0 then
            cprint("${bright green}[Command was successful]")
        else
            cprint("${bright red}[Command exited with " .. status .. "]")
        end
    end

    proc:close()
end


-- PACKAGES --------------------------------------------------------------------------
package("zethcxx.stx")
    set_kind("library", { headeronly = true })
    set_urls("https://github.com/zethcxx/stx.git")

    add_versions( "v0.2.0", "v0.2.0" )

    add_configs( "use_modules", {
        builtin = false,
        default = false,
        type    = "boolean",
        description = "Use C++ Modules"
    })

    on_install( function( package )
        local configs = {}
        local includedir = package:installdir("include")

        if package:config( "use_modules" ) then
            configs.use_modules = true
        end

        import("package.tools.xmake").install( package, configs, { includedirs = includedir })
    end)

    on_load(function (package)
        package:add("includedirs", "include")
        if package:config("use_modules") then
            package:add("cxxmodules", "modules/stx/*.cppm")
        end
    end)
package_end()

add_requires("zethcxx.stx", { configs = { use_modules = true } })
add_requires("glfw   3.4       ")
add_requires("stb    2026.03.18")
add_requires("libsdl3")
add_requires("opengl")


-- CONFIGS ---------------------------------------------------------------------------
add_rules( "vscode.compile_commands" )


target( "main" )
    set_default   ( true     )
    set_languages ( "c++23"  )
    set_kind      ( "binary" )
    set_basename  ( "window" )

    add_files("./app/main.cpp")

    add_packages(
        "zethcxx.stx",
        "stb",
        "libsdl3",
        "opengl"
    )

    set_policy( "build.c++.modules"    , true  )
    set_policy( "build.c++.modules.std", false )

    on_config( _config_proyect )
    on_run   ( _run_process    )
target_end()

