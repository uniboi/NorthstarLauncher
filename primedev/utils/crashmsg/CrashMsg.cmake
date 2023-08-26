# CrashMsg

add_executable(CrashMsg
               "tier0/filestream.cpp"
               "tier0/filestream.h"
               "util/filesystem.cpp"
			   "util/filesystem.h"
			   "util/utils.cpp"
			   "util/utils.h"
               "utils/crashmsg/crashmsg.cpp"
)

target_precompile_headers(CrashMsg PRIVATE core/stdafx.h)

target_compile_definitions(CrashMsg PRIVATE
                           UNICODE
                           _UNICODE
)

target_link_libraries(CrashMsg PRIVATE
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

target_include_directories(CrashMsg PRIVATE utils/crashmsg)

set_target_properties(CrashMsg PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${NS_BINARY_DIR}/bin
					  OUTPUT_NAME crashmsg
                      LINK_FLAGS "/MANIFEST:NO /DEBUG /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)
