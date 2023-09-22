# PrimeLauncher

add_executable(PrimeLauncher
               "logging/logging.cpp"
			   "logging/logging.h"
			   "tier0/utils.cpp"
			   "tier0/utils.h"
			   "utils/primelauncher/launcher.cpp"
			   "utils/primelauncher/launcher.h"
               "utils/primelauncher/main.cpp"
               "utils/primelauncher/resources.rc"
               "tier0/crashhandler.cpp"
               "tier0/crashhandler.h"
               "tier0/cpu.cpp"
               "tier0/cpu.h"
               "tier0/cputopology.cpp"
               "tier0/cputopology.h"
               "tier0/dbg.cpp"
               "tier0/dbg.h"
               "tier0/fasttimer.cpp"
               "tier0/fasttimer.h"
               "tier0/filestream.cpp"
               "tier0/filestream.h"
			   "windows/wconsole.cpp"
			   "windows/wconsole.h"
)

target_precompile_headers(PrimeLauncher PRIVATE core/stdafx.h)

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

target_include_directories(PrimeLauncher PRIVATE utils/primelauncher)

set_target_properties(PrimeLauncher PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${NS_BINARY_DIR}
					  OUTPUT_NAME primelauncher
                      COMPILE_FLAGS "/W4"
                      LINK_FLAGS "/MANIFEST:NO /DEBUG /STACK:8000000 /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)
