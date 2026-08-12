set(VTFLIB_SOURCES
		Error.cpp
		Float16.cpp
		MemoryReader.cpp
		MemoryWriter.cpp
		Proc.cpp
		ProcReader.cpp
		ProcWriter.cpp
		VMTFile.cpp
		VMTGroupNode.cpp
		VMTIntegerNode.cpp
		VMTNode.cpp
		VMTSingleNode.cpp
		VMTStringNode.cpp
		VMTValueNode.cpp
		VMTWrapper.cpp
        VTFDistanceAlpha.cpp
		VTFFile.cpp
		VTFLib.cpp
		VTFMathlib.cpp
		VTFWrapper.cpp
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

target_include_directories(vtflib_obj
		PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/private"
		"${CMAKE_CURRENT_SOURCE_DIR}/../shared/include"
		PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/public"
		INTERFACE
		"${CMAKE_SOURCE_DIR}/lib"
)

set_target_properties(vtflib_obj PROPERTIES
		POSITION_INDEPENDENT_CODE ON
)

add_library(vtflib SHARED)
add_library(vtflib_static STATIC)

target_link_libraries(vtflib PRIVATE vtflib_obj)
target_link_libraries(vtflib_static PRIVATE vtflib_obj)

set_target_properties(vtflib PROPERTIES
	# Deliberately not "VTFLib"!
	# third-party VTF shell extensions get loaded into our process by the shell
	# and would attempt to use our VTFLib, which has a fatal ABI mismatch
	OUTPUT_NAME VTFLib++
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
target_compile_definitions(vtflib_static
		INTERFACE VTFLIB_STATIC
)


target_include_directories(vtflib_obj PRIVATE ${VTFLIB_THIRDPARTY}/include ${VTFLIB_THIRDPARTY}/miniz)
target_link_libraries(vtflib_obj PRIVATE CMP_Compressonator miniz libzstd_static)

target_include_directories(vtflib_static PUBLIC ./../lib)


