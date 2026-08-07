find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(vtfedit WIN32
	qt/main.cpp
	qt/MainWindow.cpp
	qt/MainWindow.h
	qt/ImageView.cpp
	qt/ImageView.h
	qt/VtfOptions.cpp
	qt/VtfOptions.h
	qt/VtfOptionsDialog.cpp
	qt/VtfOptionsDialog.h
	qt/VtfFileUtility.cpp
	qt/VtfFileUtility.h
	qt/VmtFileUtility.cpp
	qt/VmtFileUtility.h
	qt/VmtEditorOptionsDialog.cpp
	qt/VmtEditorOptionsDialog.h
	qt/VmtEditorSettings.h
	qt/VmtCreateDialog.cpp
	qt/VmtCreateDialog.h
	qt/BatchConvertDialog.cpp
	qt/BatchConvertDialog.h
	qt/BatchConvertSettings.h
	qt/AboutDialog.cpp
	qt/AboutDialog.h
	qt/VTFLibQt.h
	qt/VTFEdit.rc
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
	OUTPUT_NAME VTFEdit
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

target_compile_definitions(vtfedit PRIVATE
	VTFLIB_CMAKE_BUILD
	_CRT_SECURE_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE
)

target_link_libraries(vtfedit PRIVATE
	vtflib
	DevIL
	Qt6::Core
	Qt6::Widgets
)

add_custom_command(TARGET vtfedit POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_PROPERTY:DevIL,IMPORTED_LOCATION>"
		"$<TARGET_FILE_DIR:vtfedit>"
)
