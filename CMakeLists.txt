cmake_minimum_required(VERSION 4.4)

include(GenerateExportHeader)

#set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
#set(CMAKE_XCODE_ATTRIBUTE_CODE_DEVELOPMENT_TEAM 9B39M5G274)

#set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)

#set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)

project(WonderGUI VERSION 3.0 DESCRIPTION "WonderGUI User Interface Library")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WG_ROOT ${PROJECT_SOURCE_DIR})

option(WONDERGUI_FETCH_DEPS "Automatically download SDL3, SDL2, FreeType and GLEW if not found" ON)


#------------------------------------------------------------------------------
# WONDERAPP FRAMEWORK SELECTION
#
# Select which WonderApp framework to use when building tools and other included
# applications. Pick one below, or pass -DWAPPLIB=<name> to cmake.
#
# Only the selected framework and the rendering extension it needs are built,
# so OpenGL is only required when wappsdl2gl or wappsdl3gl is selected, and SDL2
# only when one of the SDL2 frameworks is.
#
#------------------------------------------------------------------------------

if(NOT WAPPLIB)
	#set(WAPPLIB wappwin32sw)		# WonderApp framework for Win32, using software rendering.
	#set(WAPPLIB wappwin32dx12)		# WonderApp framework for Win32, using DirectX 12 rendering.

	#set(WAPPLIB wappcocoasw)		# WonderApp framework for macOS (Cocoa), using software rendering.
	#set(WAPPLIB wappcocoametal)	# WonderApp framework for macOS (Cocoa), using Metal rendering.

	#set(WAPPLIB wappsdl2sw)		# WonderApp framework based on SDL2, using software rendering.
									# Works on Windows, Mac and Linux.
	set(WAPPLIB wappsdl2gl)			# WonderApp framework based on SDL2, using OpenGL rendering.
									# Works on Windows, Mac and Linux, although deprecated on both Windows and Mac.
	#set(WAPPLIB wappsdl2metal)		# WonderApp framework based on SDL2, using Metal rendering.
									# Works on Mac.

	#set(WAPPLIB wappsdl3sw)		# WonderApp framework based on SDL3, using software rendering.
									# Works on Windows, Mac and Linux.
	#set(WAPPLIB wappsdl3gl)		# WonderApp framework based on SDL3, using OpenGL rendering.
									# Works on Windows, Mac and Linux.
	#set(WAPPLIB wappsdl3dx12)		# WonderApp framework based on SDL3, using DirectX 12 rendering.
									# Works on Windows.
	#set(WAPPLIB wappsdl3metal)		# WonderApp framework based on SDL3, using Metal rendering.
									# Works on Mac.
endif()

set(WAPPLIBS wappwin32sw wappwin32dx12 wappcocoasw wappcocoametal wappsdl2sw wappsdl2gl wappsdl2metal wappsdl3sw wappsdl3gl wappsdl3dx12 wappsdl3metal)

if(NOT WAPPLIB IN_LIST WAPPLIBS)
	message(FATAL_ERROR "Unknown WonderApp framework: ${WAPPLIB}. Choose one of: ${WAPPLIBS}")
endif()

message(STATUS "WonderApp framework: ${WAPPLIB}")


#------------------------------------------------------------------------------
# OUTPUT DIRECTORY
#
# Executables and shared libraries all go into bin/ (bin/<Config>/ with Visual
# Studio and Xcode), including SDL when it is downloaded and built here. Windows has no RPATH, so that is where they have to be for the
# executables to find them. Static libraries stay in their own directories.
#
# Set before the dependencies below, so they are built there too.
#------------------------------------------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


#------------------------------------------------------------------------------
# DEPENDENCIES
#
# SDL3 is used by the tools and examples that open their own windows, and by
# the SDL3 WonderApp frameworks. Images are loaded with stb_image, which is
# included in thirdparty/stb.
#
# The other dependencies are looked for where they are needed, so only when
# that part is built: SDL2 by the SDL2 WonderApp frameworks, FreeType by
# wax/extensions/freetypefont and OpenGL and GLEW by wax/extensions/openglgfx.
#
# The base libraries (Wax and WonderGUI) do not need any of these.
#
#------------------------------------------------------------------------------

# ---------- SDL3 ----------
find_package(SDL3 CONFIG QUIET)

if(NOT SDL3_FOUND AND WONDERGUI_FETCH_DEPS)
    include(FetchContent)

    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.4.18
        GIT_SHALLOW    TRUE
    )

    block()
        set(CMAKE_FOLDER thirdparty)
        FetchContent_MakeAvailable(SDL3)
    endblock()
endif()

# The CMakeLists below link SDL3 through this variable. It is filled in from the
# package found above, unless already given on the command line.

if(NOT SDL3_LIBS AND TARGET SDL3::SDL3)
    set(SDL3_LIBS SDL3::SDL3)
endif()

# ---------- stb_image ----------
set(STB_INCDIR ${PROJECT_SOURCE_DIR}/thirdparty/stb)


#------------------------------------------------------------------------------
# FUNCTIONS
#
# Used by subdirectories CMakeLists.
#------------------------------------------------------------------------------


function( target_working_directory target workpath )

	if(WIN32)
		set_target_properties( ${target} PROPERTIES 
			VS_DEBUGGER_WORKING_DIRECTORY ${workpath} 
		)

	elseif(APPLE)
		set_target_properties( ${target} PROPERTIES
			XCODE_GENERATE_SCHEME TRUE
			XCODE_SCHEME_WORKING_DIRECTORY "${workpath}"
		)
	endif()

endfunction()


#------------------------------------------------------------------------------
# COMPILER OPTIONS
#------------------------------------------------------------------------------

if(MSVC)
	add_compile_options("/MP")
endif()

#------------------------------------------------------------------------------
# SUBDIRECTORIES
#
# CMAKE_FOLDER puts the targets of each subdirectory in the IDE folder (Visual
# Studio solution folder, Xcode group) that matches where it is on disk.
# Dependencies that are downloaded go into the thirdparty folder.
#------------------------------------------------------------------------------

set(CMAKE_FOLDER wax)
add_subdirectory(wax)

set(CMAKE_FOLDER wondergui)
add_subdirectory(wondergui)

set(CMAKE_FOLDER wondergui/extensions)
add_subdirectory(wondergui/extensions/debugger)

set(CMAKE_FOLDER wax/extensions)

if( WAPPLIB STREQUAL "wappsdl2metal" OR WAPPLIB STREQUAL "wappsdl3metal" OR WAPPLIB STREQUAL "wappcocoametal" )
	add_subdirectory(wax/extensions/metalgfx)
elseif( WAPPLIB STREQUAL "wappwin32dx12" OR WAPPLIB STREQUAL "wappsdl3dx12" )
	add_subdirectory(wax/extensions/dx12gfx)
elseif( WAPPLIB STREQUAL "wappsdl2gl" OR WAPPLIB STREQUAL "wappsdl3gl" )
	add_subdirectory(wax/extensions/openglgfx)
endif()

add_subdirectory(wax/extensions/gfxstream)
add_subdirectory(wax/extensions/softgfx)

add_subdirectory(wax/extensions/freetypefont)

set(CMAKE_FOLDER wax/tools)
add_subdirectory(wax/tools)

set(CMAKE_FOLDER wondergui/extensions)
add_subdirectory(wondergui/extensions/legacygui)
add_subdirectory(wondergui/extensions/plugin)

set(CMAKE_FOLDER wonderapp)

if( WAPPLIB STREQUAL "wappwin32sw" )
	add_subdirectory(wonderapp/source/win32/sw)
elseif( WAPPLIB STREQUAL "wappwin32dx12" )
	add_subdirectory(wonderapp/source/win32/dx12)
elseif( WAPPLIB STREQUAL "wappcocoasw" )
	add_subdirectory(wonderapp/source/cocoa/sw)
elseif( WAPPLIB STREQUAL "wappcocoametal" )
	add_subdirectory(wonderapp/source/cocoa/metal)
elseif( WAPPLIB MATCHES "^wappsdl3" )
	add_subdirectory(wonderapp/source/sdl3)
else()
	add_subdirectory(wonderapp/source/sdl2)
endif()

set(CMAKE_FOLDER wonderapp/tools)
add_subdirectory(wonderapp/tools)

set(CMAKE_FOLDER wonderapp/examples)
add_subdirectory(wonderapp/examples)

set(CMAKE_FOLDER wondergui/tools)
add_subdirectory(wondergui/tools)

set(CMAKE_FOLDER wondergui/examples)
add_subdirectory(wondergui/examples)

unset(CMAKE_FOLDER)


#install(TARGETS example01 DESTINATION ${WG_ROOT}/distribution/examples)

install(TARGETS streamanalyzer DESTINATION ${WG_ROOT}/distribution/tools)
