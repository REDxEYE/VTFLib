include(FetchContent)

if (NOT TARGET CMP_Core)

    FetchContent_Declare(
            compressonator
            QUIET
            GIT_REPOSITORY https://github.com/REDxEYE/compressonator.git
            GIT_TAG master
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
    set(OPTION_BUILD_BROTLIG OFF CACHE BOOL "" FORCE)
    set(OPTION_BUILD_INTERNAL_CMP_TEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(compressonator)

endif ()

if (NOT TARGET IL)

    FetchContent_Declare(
            DevIL
            QUIET
            GIT_REPOSITORY https://github.com/REDxEYE/DevIL.git
            GIT_TAG master
            SOURCE_SUBDIR DevIL/src-IL
            GIT_SHALLOW TRUE

    )

    set(OPTION_BUILD_INTERNAL_CMP_TEST OFF CACHE BOOL "" FORCE)
#    set(IL_STATIC_DEPS ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(devil)

    target_include_directories(IL PUBLIC
            "${devil_SOURCE_DIR}/DevIL/include"
    )

    if (WIN32)
        target_compile_definitions(IL PUBLIC IL_STATIC_LIB)
    endif ()
endif ()

if (NOT TARGET miniz)

    FetchContent_Declare(
            miniz
            QUIET
            GIT_REPOSITORY https://github.com/richgel999/miniz.git
            GIT_TAG 3.1.2
            GIT_SHALLOW TRUE

    )

    FetchContent_MakeAvailable(miniz)
endif ()

if (NOT TARGET libzstd_static)

    FetchContent_Declare(
            zstd
            QUIET
            GIT_REPOSITORY https://github.com/facebook/zstd.git
            GIT_TAG v1.5.7
            SOURCE_SUBDIR build/cmake
            GIT_SHALLOW TRUE

    )
    set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(zstd)
endif ()

set_target_properties(
        miniz
        libzstd_static
        CMP_Compressonator
        CMP_Core
        PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

if (MSVC)
    foreach (target
            CMP_Compressonator
            CMP_Core
            CMP_Core_SSE
            CMP_Core_AVX
            CMP_Core_AVX512
    )
        if (TARGET ${target})
            set_property(
                    TARGET ${target}
                    PROPERTY MSVC_RUNTIME_LIBRARY
                    "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            )
        endif ()
    endforeach ()
endif ()