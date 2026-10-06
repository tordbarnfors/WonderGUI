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

option(WONDERGUI_FETCH_DEPS "Automatically download SDL2/FreeType if not found" ON)


#------------------------------------------------------------------------------
# DIRECTORIES
#
# SDL2, SDL2_image, GLEW and FreeType libraries are needed by several compile targets,
# such as tools, workbenches, examples and some WonderApp frameworks.
#
# The base libraries (WonderGear, WonderGfx, WonderGlyphs and WonderGUI) do not need these.
#
#------------------------------------------------------------------------------

# ---------- SDL2 ----------
find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND AND WONDERGUI_FETCH_DEPS)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-2.30.0
    )
    FetchContent_MakeAvailable(SDL2)
    # SDL2 sets up SDL2::SDL2 / SDL2::SDL2main targets itself when built this way
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
# WONDERAPP FRAMEWORK SELECTION
#
# Select which WonderApp framework to use when building tools and other included
# applications.
#
#------------------------------------------------------------------------------

#set(WAPPLIB wappwin32sw)			# WonderApp framework for Win32, using software rendering.
set(WAPPLIB wappwin32dx12)			# WonderApp framework for Win32, using software rendering.

#set(WAPPLIB wappsdl2sw)			# WonderApp framework based on SDL2, using software rendering.
									# Works on Windows, Mac and Linux.
set(WAPPLIB wappsdl2gl)			# WonderApp framework based on SDL2, using OpenGL rendering.
									# Works on Windows, Mac and Linux, although deprecated on both Windows and Mac.
#set(WAPPLIB wappsdl2metal)			# WonderApp framework based on SDL2, using Metal rendering.
									# Works on Mac.


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

if(APPLE)
	add_subdirectory(extensions/metalgfx)
endif()

if(WIN32)
	add_subdirectory(extensions/dx12gfx)
endif()

add_subdirectory(extensions/openglgfx)
add_subdirectory(extensions/gfxstream)
add_subdirectory(extensions/softgfx)

add_subdirectory(extensions/freetypefont)

#add_subdirectory(extensions/legacygui)

#add_subdirectory(extensions/plugin)

if( WAPPLIB STREQUAL "wappwin32sw" )
	add_subdirectory(wonderapp/win32-sw)
elseif( WAPPLIB STREQUAL "wappwin32dx12" )
	add_subdirectory(wonderapp/win32-dx12)
elseif( WAPPLIB STREQUAL "wappsdl2sw" OR WAPPLIB STREQUAL "wappsdl2gl" OR WAPPLIB STREQUAL "wappsdl2metal" )
	add_subdirectory(wonderapp/wappsdl2)
else()
	message(FATAL_ERROR "Unknown WonderApp framework: ${WAPPLIB}")
endif()

add_subdirectory(tools)
add_subdirectory(examples)


#install(TARGETS example01 DESTINATION ${WG_ROOT}/distribution/examples)

install(TARGETS streamanalyzer DESTINATION ${WG_ROOT}/distribution/tools)
