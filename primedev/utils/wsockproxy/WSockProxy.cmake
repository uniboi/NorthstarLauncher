# WSockProxy

find_package(minhook REQUIRED)

add_library(WSockProxy SHARED
            "logging/logging.cpp"
			"logging/logging.h"
			"tier0/utils.cpp"
            "tier0/utils.h"
			"utils/primelauncher/launcher.cpp"
			"utils/primelauncher/launcher.h"
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
            "utils/wsockproxy/dllmain.cpp"
            "utils/wsockproxy/proxy.cpp"
			"windows/wconsole.cpp"
			"windows/wconsole.h"
            "utils/wsockproxy/WSockProxy.def"
)

target_precompile_headers(WSockProxy PRIVATE core/stdafx.h)

target_link_libraries(WSockProxy PRIVATE
                      minhook
                      mswsock.lib
                      ws2_32.lib
                      ShLwApi.lib
                      imagehlp.lib
                      dbghelp.lib
                      kernel32.lib
                      user32.lib
                      gdi32.lib
                      winspool.lib
                      comdlg32.lib
                      advapi32.lib
                      shell32.lib
                      ole32.lib
                      oleaut32.lib
                      uuid.lib
                      odbc32.lib
                      odbccp32.lib
)

target_include_directories(WSockProxy PRIVATE utils/wsockproxy utils)

target_compile_definitions(WSockProxy PRIVATE
                           WSOCKPROXY
                           UNICODE
                           _UNICODE
)

set_target_properties(WSockProxy PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${NS_BINARY_DIR}/bin/x64_retail
                      OUTPUT_NAME wsock32
                      COMPILE_FLAGS "/W4"
                      LINK_FLAGS "/MANIFEST:NO /DEBUG"
)
