# loader_wsock32_proxy

find_package(minhook REQUIRED)

add_library(loader_wsock32_proxy SHARED
            "logging/logging.cpp"
			"logging/logging.h"
            "util/filesystem.cpp"
			"util/filesystem.h"
			"util/utils.cpp"
            "util/utils.h"
			"primelauncher/launcher.cpp"
			"primelauncher/launcher.h"
			"tier0/crashhandler.cpp"
			"tier0/crashhandler.h"
			"tier0/dbg.cpp"
			"tier0/dbg.h"
            "wsockproxy/dllmain.cpp"
            "wsockproxy/proxy.cpp"
			"windows/wconsole.cpp"
			"windows/wconsole.h"
            "WSockProxy.def"
)

target_precompile_headers(loader_wsock32_proxy PRIVATE core/stdafx.h)

target_link_libraries(loader_wsock32_proxy PRIVATE
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

target_compile_definitions(loader_wsock32_proxy PRIVATE
                           WSOCKPROXY
                           UNICODE
                           _UNICODE
)

set_target_properties(loader_wsock32_proxy PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${NS_BINARY_DIR}/bin/x64_retail
                      OUTPUT_NAME wsock32
                      LINK_FLAGS "/MANIFEST:NO /DEBUG"
)
