find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(vtfedit WIN32
        main.cpp
        MainWindow.cpp
        MainWindow.h
        ImageView.cpp
        ImageView.h
        SheetFile.cpp
        SheetFile.h
        SheetDialog.cpp
        SheetDialog.h
        VtfOptions.cpp
        VtfOptions.h
        VtfOptionsDialog.cpp
        VtfOptionsDialog.h
        VtfFileUtility.cpp
        VtfFileUtility.h
        VmtFileUtility.cpp
        VmtFileUtility.h
        VmtEditorOptionsDialog.cpp
        VmtEditorOptionsDialog.h
        VmtEditorSettings.h
        VmtTextEdit.cpp
        VmtTextEdit.h
        VmtCreateDialog.cpp
        VmtCreateDialog.h
        BatchConvertDialog.cpp
        BatchConvertDialog.h
        BatchConvertSettings.h
        AboutDialog.cpp
        AboutDialog.h
        VTFLibQt.h
        VTFEdit.rc
        VTFEditHelpers.cpp
)

qt_add_resources(vtfedit "vtfedit_resources"
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES
        app.ico
        icons/rgb.png
        icons/red.png
        icons/green.png
        icons/blue.png
        icons/alpha.png
        icons/alphamask.png
        icons/tile.png
)

set_target_properties(vtfedit PROPERTIES
        OUTPUT_NAME VTFEdit++
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
        INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON
        INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON
)

target_compile_definitions(vtfedit PRIVATE
        VTFLIB_CMAKE_BUILD
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
)

set(_indirect_extern_flag)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(_indirect_extern_flag "-fno-direct-access-external-data")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(_indirect_extern_flag "-mno-direct-extern-access -fno-plt")
endif ()

if (_indirect_extern_flag)
    check_cxx_compiler_flag(
            "${_indirect_extern_flag}"
            VTFEDIT_HAS_INDIRECT_EXTERN_ACCESS
    )

    if (VTFEDIT_HAS_INDIRECT_EXTERN_ACCESS)
        target_compile_options(
                vtfedit
                PRIVATE
                "${_indirect_extern_flag}"
        )
    endif ()
endif ()
target_link_libraries(vtfedit PRIVATE
        vtflib_static
        CMP_Compressonator
        CMP_Framework
        CMP_Core
        Qt6::Core
        Qt6::Widgets
)