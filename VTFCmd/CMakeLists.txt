set(VTFCMD_SOURCES
		vtfcmd.cpp
		enumerations.cpp
)

if(WIN32)
	list(APPEND VTFCMD_SOURCES
			VTFCmd.rc
	)
endif()

add_executable(vtfcmd ${VTFCMD_SOURCES})

set_target_properties(vtfcmd PROPERTIES
	OUTPUT_NAME VTFCmd
)

target_include_directories(vtfcmd
		PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"../shared/include"
)

target_link_libraries(vtfcmd PRIVATE vtflib_static CMP_Compressonator CMP_Framework CMP_Core)