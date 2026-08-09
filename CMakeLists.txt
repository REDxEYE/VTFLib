cmake_minimum_required(VERSION 3.21)

project(VTFLib
        VERSION 2.0.0
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(
            CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            CACHE STRING ""
            FORCE
    )
endif()

include(cmake/dependencies.cmake)

option(VTFLIB_BUILD_VTFCMD "Build VTFCmd" ON)
option(VTFLIB_BUILD_VTFEDIT "Build VTFEdit" ON)


if (NOT WIN32)
    message(WARNING "VTFCMD and VTFEDIT only supported on Windows")
    SET(VTFLIB_BUILD_VTFEDIT OFF)
    SET(VTFLIB_BUILD_VTFCMD OFF)
endif ()

foreach (config "" _DEBUG _RELEASE _RELWITHDEBINFO _MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${config} "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY${config} "${CMAKE_BINARY_DIR}/bin")
endforeach ()

set(VTFLIB_THIRDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

#add_library(DevIL SHARED IMPORTED GLOBAL)
#set_target_properties(DevIL PROPERTIES
#	IMPORTED_IMPLIB "${VTFLIB_THIRDPARTY}/lib/DevIL.lib"
#	IMPORTED_LOCATION "${VTFLIB_THIRDPARTY}/bin/DevIL.dll"
#	INTERFACE_INCLUDE_DIRECTORIES "${VTFLIB_THIRDPARTY}/include"
#)

add_subdirectory(VTFLib)

if (VTFLIB_BUILD_VTFCMD)
    add_subdirectory(VTFCmd)
endif ()

if (VTFLIB_BUILD_VTFEDIT)
    add_subdirectory(VTFEdit)
endif ()

