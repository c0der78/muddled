--_ACTION = _ACTION or "arg3"
newaction {
   trigger     = "indent",
   description = "Format source files",
   execute = function ()
      os.execute("astyle -N -A1 -R *.c *.h");
   end
}

newaction {
    trigger     = "doc",
    description = "Document source files",
    execute = function()
         os.execute("headerdoc2html -j -o docs src/muddyplains/*.h")
         os.execute("gatherheaderdoc docs index.html")
    end
}

if _ACTION == "clean" then
    matches = os.matchfiles("**.orig")
    for i=1, #matches do
        os.remove(matches[i])
    end
end

solution "muddy"
    configurations { "Debug", "Release" }
    language "C"

    buildoptions { "-std=c99", "-I`pwd`/src", "-DROOT_DIR=`pwd`", "-Wall", "-Werror"}

    configuration "Debug"
        flags "Symbols"
        targetdir "bin/debug"

    configuration "Release"
        targetdir "bin/release"

    project "muddy"
        kind "StaticLib"
        files {
            "src/muddyengine/**.c",
            "src/muddyengine/**.h"
        }

    project "muddy_server"
        kind "StaticLib"
        files {
            "src/muddyplains/**.c",
            "src/muddyplains/**.h",
            "src/config.h"
        }

    project "muddyplains"
        kind "ConsoleApp"
        files {
            "src/main.c",
            "src/config.h",
        }
        links { "sqlite3", "muddy", "muddy_server", "lua"}
    
    project "muddyengine_test"
        kind "ConsoleApp"
        files {
            "tests/muddyengine/**.c"
        }
        
        links { "sqlite3", "muddy", "lua", "check" }
        
        configuration "Debug"
            postbuildcommands {
                "bin/debug/muddyengine_test"
            }
        configuration "Release"
            postbuildcommands {
                "bin/release/muddyengine_test"
            }

    project "muddyplains_test"
        kind "ConsoleApp"
        files {
            "tests/muddyplains/**.c",
        }

        links { "sqlite3", "muddy_server", "muddy", "lua", "check" }

        configuration "Debug"
            postbuildcommands {
                "bin/debug/muddyplains_test"
            }

        configuration "Release"
            postbuildcommands {
                "bin/release/muddyplains_test"
            }