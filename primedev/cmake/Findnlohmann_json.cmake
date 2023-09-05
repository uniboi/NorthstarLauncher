
if(NOT nlohmann_json_FOUND)
	check_init_submodule(${PROJECT_SOURCE_DIR}/primedev/thirdparty/nlohmann)

	add_subdirectory(${PROJECT_SOURCE_DIR}/primedev/thirdparty/nlohmann nlohmann_json)
	set(nlohmann_json_FOUND 1)
endif()
