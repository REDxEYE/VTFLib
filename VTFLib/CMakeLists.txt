include(FetchContent)

set(VTFLIB_SOURCES
		Error.cpp
		Float16.cpp
		MemoryReader.cpp
		MemoryWriter.cpp
		VMTFile.cpp
		VMTGroupNode.cpp
		VMTIntegerNode.cpp
		VMTNode.cpp
		VMTSingleNode.cpp
		VMTStringNode.cpp
		VMTValueNode.cpp
		VMTWrapper.cpp
		VTFFile.cpp
		VTFLib.cpp
		VTFMathlib.cpp
		VTFWrapper.cpp
		Proc.cpp
		ProcReader.cpp
		ProcWriter.cpp

		"${VTFLIB_THIRDPARTY}/miniz/miniz.c"
		"${VTFLIB_THIRDPARTY}/zstd/zstd.c"
)

if(WIN32)
	list(APPEND VTFLIB_SOURCES
			FileReader.cpp
			FileWriter.cpp
			Resource.rc
	)
else()
	list(APPEND VTFLIB_SOURCES
			FIleReader_linux.cpp
			FileWriter_linux.cpp
	)
endif()

add_library(vtflib_obj OBJECT ${VTFLIB_SOURCES})

set_target_properties(vtflib_obj PROPERTIES
		POSITION_INDEPENDENT_CODE ON
)

add_library(vtflib SHARED
		$<TARGET_OBJECTS:vtflib_obj>
)

add_library(vtflib_static STATIC
		$<TARGET_OBJECTS:vtflib_obj>
)

set_target_properties(vtflib PROPERTIES
	# Deliberately not "VTFLib"!
	# third-party VTF shell extensions get loaded into our process by the shell
	# and would attempt to use our VTFLib, which has a fatal ABI mismatch
	OUTPUT_NAME VTFLib++
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_definitions(vtflib_obj
	PRIVATE
		VTFLIB_EXPORTS
		_WINDOWS
		_USRDLL
		_CRT_SECURE_NO_WARNINGS
		_CRT_NONSTDC_NO_DEPRECATE
		$<$<NOT:$<CONFIG:Debug>>:_USE_RTM_VERSION>
)

target_include_directories(vtflib_obj
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${VTFLIB_THIRDPARTY}/include"
		"${VTFLIB_THIRDPARTY}/miniz"
		"${VTFLIB_THIRDPARTY}/zstd"
	INTERFACE
		"${CMAKE_SOURCE_DIR}/lib"
)

if(WIN32)
	target_link_libraries(vtflib_obj PRIVATE
			"$<IF:$<CONFIG:Debug>,${VTFLIB_THIRDPARTY}/lib/Compressonator_MTd.lib,${VTFLIB_THIRDPARTY}/lib/Compressonator_MT.lib>"
	)
else()
	target_include_directories(vtflib_obj PRIVATE ${VTFLIB_THIRDPARTY}/include ${VTFLIB_THIRDPARTY}/miniz)
	FetchContent_Declare(
			compressonator
			QUIET
			GIT_REPOSITORY https://github.com/GPUOpen-Tools/compressonator.git
			GIT_TAG f4b53d7
			GIT_SHALLOW TRUE

	)

	set(OPTION_ENABLE_ALL_APPS OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_CMP_SDK ON CACHE BOOL "" FORCE)

	set(OPTION_BUILD_APPS_CMP_CLI OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_APPS_CMP_GUI OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_APPS_CMP_UNITTESTS OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_APPS_CMP_EXAMPLES OFF CACHE BOOL "" FORCE)

	set(OPTION_CMP_QT OFF CACHE BOOL "" FORCE)
	set(OPTION_CMP_OPENGL OFF CACHE BOOL "" FORCE)
	set(OPTION_CMP_OPENCV OFF CACHE BOOL "" FORCE)
	set(OPTION_CMP_VULKAN OFF CACHE BOOL "" FORCE)

	set(OPTION_BUILD_KTX2 OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_EXR OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_INTERNAL_CMP_TEST OFF CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(compressonator)

	target_link_libraries(vtflib_obj PRIVATE CMP_Compressonator )
endif ()



