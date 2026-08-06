add_library(vtflib SHARED
	Error.cpp
	FileReader.cpp
	FileWriter.cpp
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
	VTFFile.cpp
	VTFLib.cpp
	VTFMathlib.cpp
	VTFWrapper.cpp
	Resource.rc
	"${VTFLIB_THIRDPARTY}/miniz/miniz.c"
	"${VTFLIB_THIRDPARTY}/zstd/zstd.c"
)

set_target_properties(vtflib PROPERTIES
	OUTPUT_NAME VTFLib
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_definitions(vtflib
	PRIVATE
		VTFLIB_EXPORTS
		_WINDOWS
		_USRDLL
		_CRT_SECURE_NO_WARNINGS
		_CRT_NONSTDC_NO_DEPRECATE
		$<$<NOT:$<CONFIG:Debug>>:_USE_RTM_VERSION>
)

target_include_directories(vtflib
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${VTFLIB_THIRDPARTY}/include"
		"${VTFLIB_THIRDPARTY}/miniz"
		"${VTFLIB_THIRDPARTY}/zstd"
	INTERFACE
		"${CMAKE_SOURCE_DIR}/lib"
)

target_link_libraries(vtflib PRIVATE
	"$<IF:$<CONFIG:Debug>,${VTFLIB_THIRDPARTY}/lib/Compressonator_MTd.lib,${VTFLIB_THIRDPARTY}/lib/Compressonator_MT.lib>"
)
