# PrimeLauncher

add_executable(PrimeLauncher
               "util/filesystem.cpp"
			   "util/filesystem.h"
			   "util/utils.cpp"
			   "util/utils.h"
			   "primelauncher/launcher.cpp"
			   "primelauncher/launcher.h"
               "primelauncher/main.cpp"
               "primelauncher/resources.rc"
               "tier0/crashhandler.cpp"
               "tier0/crashhandler.h"
)

target_compile_definitions(PrimeLauncher PRIVATE
                           LAUNCHER
                           UNICODE
                           _UNICODE
)

target_link_libraries(PrimeLauncher PRIVATE
                      shlwapi.lib
                      kernel32.lib
                      user32.lib
                      gdi32.lib
                      winspool.lib
					  dbghelp.lib
                      comdlg32.lib
                      advapi32.lib
                      shell32.lib
                      ole32.lib
                      oleaut32.lib
                      uuid.lib
                      odbc32.lib
                      odbccp32.lib
                      WS2_32.lib
)

set_target_properties(PrimeLauncher PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${NS_BINARY_DIR}
					  OUTPUT_NAME r2primelauncher
                      LINK_FLAGS "/MANIFEST:NO /DEBUG /STACK:8000000"
)
