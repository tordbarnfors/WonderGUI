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

set(WG_ROOT ${PROJECT_SOURCE_DIR}/..)

option(WONDERGUI_FETCH_DEPS "Automatically download SDL2, SDL2_image, FreeType and GLEW if not found" ON)


#------------------------------------------------------------------------------
# WONDERAPP FRAMEWORK SELECTION
#
# Select which WonderApp framework to use when building tools and other included
# applications. Pick one below, or pass -DWAPPLIB=<name> to cmake.
#
# Only the selected framework and the rendering extension it needs are built,
# so OpenGL is only required when wappsdl2gl is selected.
#
#------------------------------------------------------------------------------

if(NOT WAPPLIB)
	#set(WAPPLIB wappwin32sw)		# WonderApp framework for Win32, using software rendering.
	#set(WAPPLIB wappwin32dx12)		# WonderApp framework for Win32, using DirectX 12 rendering.

	#set(WAPPLIB wappsdl2sw)		# WonderApp framework based on SDL2, using software rendering.
									# Works on Windows, Mac and Linux.
	set(WAPPLIB wappsdl2gl)			# WonderApp framework based on SDL2, using OpenGL rendering.
									# Works on Windows, Mac and Linux, although deprecated on both Windows and Mac.
	#set(WAPPLIB wappsdl2metal)		# WonderApp framework based on SDL2, using Metal rendering.
									# Works on Mac.
endif()

set(WAPPLIBS wappwin32sw wappwin32dx12 wappsdl2sw wappsdl2gl wappsdl2metal)

if(NOT WAPPLIB IN_LIST WAPPLIBS)
	message(FATAL_ERROR "Unknown WonderApp framework: ${WAPPLIB}. Choose one of: ${WAPPLIBS}")
endif()

message(STATUS "WonderApp framework: ${WAPPLIB}")


#------------------------------------------------------------------------------
# DIRECTORIES
#
# SDL2, SDL2_image and FreeType libraries are needed by several compile targets,
# such as tools, workbenches, examples and some WonderApp frameworks. OpenGL and
# GLEW are looked for by extensions/openglgfx, which is only built for wappsdl2gl.
#
# The base libraries (WonderGear, WonderGfx, WonderGlyphs and WonderGUI) do not need these.
#
#------------------------------------------------------------------------------

# ---------- SDL2 ----------
find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND AND WONDERGUI_FETCH_DEPS)
    include(FetchContent)

    # With MSVC, SDL2 builds without the C runtime by default and links its DLL
    # with /NODEFAULTLIB. It keeps runtime checks (/RTC1), which need the C
    # runtime, out by removing them from CMAKE_C_FLAGS_DEBUG. CMake 4 (policy
    # CMP0184, NEW for everything since we require 4.4) adds them through the
    # MSVC_RUNTIME_CHECKS target property instead, so they stay and the link
    # fails on _RTC_CheckStackVars. Building SDL2 with the C runtime avoids it.

    set(SDL_LIBC ON CACHE BOOL "Use the system C library" FORCE)

    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-2.30.0
    )
    FetchContent_MakeAvailable(SDL2)
    # SDL2 sets up SDL2::SDL2 / SDL2::SDL2main targets itself when built this way
endif()

# ---------- SDL2_image ----------
find_package(SDL2_image CONFIG QUIET)

if(NOT SDL2_image_FOUND AND WONDERGUI_FETCH_DEPS)
    include(FetchContent)

    # PNG and JPEG through SDL_image's built-in stb backend is all we need, so
    # none of its third-party codecs, and none of the submodules they live in.

    set(SDL2IMAGE_VENDORED OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
    set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_JXL OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_TIF OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG        release-2.8.8
        GIT_SHALLOW    TRUE
        GIT_SUBMODULES ""
    )
    FetchContent_MakeAvailable(SDL2_image)
endif()

# The CMakeLists below link SDL through these variables. They are filled in from
# the packages found above, unless already given on the command line.
#
# SDL.h turns main() into SDL_main() on Windows, which SDL2main then calls.

if(NOT SDL2_LIBS AND TARGET SDL2::SDL2)
    set(SDL2_LIBS SDL2::SDL2)
    if(WIN32 AND TARGET SDL2::SDL2main)
        set(SDL2_LIBS SDL2::SDL2main SDL2::SDL2)
    endif()
endif()

if(NOT SDL2IMAGE_LIBS AND TARGET SDL2_image::SDL2_image)
    set(SDL2IMAGE_LIBS SDL2_image::SDL2_image)
endif()

# ---------- FreeType ----------
find_package(Freetype QUIET)

if(NOT Freetype_FOUND AND WONDERGUI_FETCH_DEPS)
    include(FetchContent)
    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG        VER-2-14-3
    )
    FetchContent_MakeAvailable(freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()







#    find_library(SDL2_LIBS SDL2)
#    find_library(SDL2IMAGE_LIBS SDL2_image)
#    find_library(OPENGL_LIBS OpenGL)
#    find_library(APPKIT_LIB AppKit)
#    find_library(METAL_LIB Metal)



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

add_subdirectory(wondergear)
add_subdirectory(wondergfx)
add_subdirectory(wondergui)

add_subdirectory(extensions/debugger)

if( WAPPLIB STREQUAL "wappsdl2metal")
	add_subdirectory(extensions/metalgfx)
elseif( WAPPLIB STREQUAL "wappwin32dx12" )
	add_subdirectory(extensions/dx12gfx)
elseif( WAPPLIB STREQUAL "wappsdl2gl" )
	add_subdirectory(extensions/openglgfx)
endif()

add_subdirectory(extensions/gfxstream)
add_subdirectory(extensions/softgfx)

add_subdirectory(extensions/freetypefont)

#add_subdirectory(extensions/legacygui)

#add_subdirectory(extensions/plugin)

if( WAPPLIB STREQUAL "wappwin32sw" )
	add_subdirectory(wonderapp/win32-sw)
elseif( WAPPLIB STREQUAL "wappwin32dx12" )
	add_subdirectory(wonderapp/win32-dx12)
else()
	add_subdirectory(wonderapp/wappsdl2)
endif()

add_subdirectory(tools)
add_subdirectory(examples)


#install(TARGETS example01 DESTINATION ${WG_ROOT}/distribution/examples)

install(TARGETS streamanalyzer DESTINATION ${WG_ROOT}/distribution/tools)
