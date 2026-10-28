set(CMAKE_CXX_STANDARD 20)

if(NOT SDL3_LIBS)
    message(FATAL_ERROR "SDL3 not found. Install it, or enable WONDERGUI_FETCH_DEPS to download it.")
endif()


set(SOURCES

	main.cpp
    sdlwindow.h
	sdlwindow.cpp
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs/tinyfiledialogs.h
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs/tinyfiledialogs.c
	../wappapi.h
	../wappwindow.h
	../wappwindow.cpp
	../wappimageloader.h
	../wappimageloader.cpp
    ../wappwindowapi.h
    ../wonderapp.h
)

set(INCDIRS
	${CMAKE_CURRENT_LIST_DIR}
	${WONDERGUI_INCDIRS}
	${WAXFREETYPEFONT_INCDIRS}
	${WGDEBUGGER_INCDIRS}
	${PROJECT_SOURCE_DIR}/wonderapp/source
	${STB_INCDIR}
	${PROJECT_SOURCE_DIR}/thirdparty/tinyfiledialogs
	${PROJECT_SOURCE_DIR}
)

# Only the selected framework gets a target, so nothing here needs OpenGL,
# DirectX 12 or Metal unless that is what was asked for.

### Software rendering version

if(WAPPLIB STREQUAL "wappsdl3sw")

	add_library(wappsdl3sw STATIC
		${SOURCES}
		backend_software.cpp
	)

	target_include_directories(wappsdl3sw PRIVATE
		${INCDIRS}
		${WAXSOFTGFX_INCDIRS}
	)

	target_link_libraries(wappsdl3sw PRIVATE
		wgdebugger
		waxsoftgfx
		waxfreetypefont
		wondergui
		${SDL3_LIBS}
	)

endif()

### OpenGL version

if(WAPPLIB STREQUAL "wappsdl3gl")

	add_library(wappsdl3gl STATIC
		${SOURCES}
		backend_opengl.cpp
	)

	target_include_directories(wappsdl3gl PRIVATE
		${INCDIRS}
		${WAXOPENGLGFX_INCDIRS}
	)

	target_link_libraries(wappsdl3gl PRIVATE
		waxopenglgfx
		waxfreetypefont
		wondergui
		wgdebugger
		${SDL3_LIBS}
	)

endif()

### DirectX 12 version

if(WAPPLIB STREQUAL "wappsdl3dx12")

	if(NOT WIN32)
		message(FATAL_ERROR "wappsdl3dx12 is only available on Windows.")
	endif()

	# Uses the same D3D12 device wrapper as the Win32 DX12 WonderApp.

	add_library(wappsdl3dx12 STATIC
		${SOURCES}
		backend_dx12.cpp
		../win32/dx12/dx12_wrapper.h
		../win32/dx12/dx12_wrapper.cpp
	)

	target_include_directories(wappsdl3dx12 PRIVATE
		${INCDIRS}
		${DX12GFX_INCDIRS}
		${PROJECT_SOURCE_DIR}/wonderapp/source/win32/dx12
	)

	target_link_libraries(wappsdl3dx12 PRIVATE
		waxdx12gfx
		waxfreetypefont
		wondergui
		wgdebugger
		${SDL3_LIBS}
	)

endif()

### Metal version

if(WAPPLIB STREQUAL "wappsdl3metal")

	if(NOT APPLE)
		message(FATAL_ERROR "wappsdl3metal is only available on macOS.")
	endif()

	add_library(wappsdl3metal STATIC
		${SOURCES}
		backend_metal.mm
	)

	target_include_directories(wappsdl3metal PRIVATE
		${INCDIRS}
		${WAXMETALGFX_INCDIRS}
	)

	target_link_libraries(wappsdl3metal PRIVATE
		waxmetalgfx
		waxfreetypefont
		wondergui
		wgdebugger
		${SDL3_LIBS}
		"-framework Metal"
		"-framework QuartzCore"
	)

endif()


set(WAPP_SOURCES ${SOURCES} PARENT_SCOPE)
set(WAPP_INCDIRS ${INCDIRS} PARENT_SCOPE)


# Show the files the way they are laid out in wonderapp/source, with the third
# party ones in a group of their own.

get_target_property(WAPP_FILES ${WAPPLIB} SOURCES)
set(THIRDPARTY_FILES ${WAPP_FILES})
list(FILTER WAPP_FILES EXCLUDE REGEX "/thirdparty/")
list(FILTER THIRDPARTY_FILES INCLUDE REGEX "/thirdparty/")

source_group(TREE ${PROJECT_SOURCE_DIR}/wonderapp/source FILES ${WAPP_FILES})
source_group(thirdparty FILES ${THIRDPARTY_FILES})
