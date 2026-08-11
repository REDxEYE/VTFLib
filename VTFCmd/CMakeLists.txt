set(VTFCMD_SOURCES
		vtfcmd.cpp
		enumerations.cpp
)

if(WIN32)
	list(APPEND VTFCMD_SOURCES
			VTFCmd.rc
	)
else()

endif()

add_executable(vtfcmd ${VTFCMD_SOURCES})



set_target_properties(vtfcmd PROPERTIES
	OUTPUT_NAME VTFCmd
)

target_compile_definitions(vtfcmd PRIVATE
	VTFLIB_CMAKE_BUILD
	_CONSOLE
	_CRT_SECURE_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE
	$<$<NOT:$<CONFIG:Debug>>:_USE_RTM_VERSION>
)
target_include_directories(vtfcmd
		PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"../shared/include"
		"../lib"
		"${devil_SOURCE_DIR}/DevIL/include"
)

target_link_libraries(vtfcmd PRIVATE vtflib_static IL)